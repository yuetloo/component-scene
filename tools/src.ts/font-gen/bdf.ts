// See: https://adobe-type-tools.github.io/font-tech-notes/pdfs/5005.BDF_Spec.pdf

const skip = "COMMENT ENDCHAR ENDFONT ENDPROPERTIES".split(" ");

export class Box {
    readonly x: number;
    readonly y: number;

    readonly width: number;
    readonly height: number;

    constructor(width: number, height: number, x: number, y: number) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    toString(): string {
        return `<Bounds x=${ this.x } y=${ this.y } width=${ this.width } height=${ this.height }`;
    }

    static fromBbx(bbx: string): Box {
        const values = <[ number, number, number, number ]>bbx.split(/\s+/).map(x => (parseInt(x.trim())));
        // @TODO: chck 4 numbes
        return new Box(...values);
    }
}

function emptyScanline(width: number): Array<0> {
    const result: Array<0> = [ ];
    while (result.length < width) { result.push(0); }
    return result;
}

export class Bitmap {
    #data: Array<Array<0 | 1>>

    readonly width: number;
    readonly height: number;

    constructor(data: Array<Array<0 | 1>>) {
        if (data.length === 0) { throw new Error("no data"); }
        data.reduce((a, r) => {
            if (a === -1) { return r.length; }
            if (a !== r.length) { throw new Error("invalid row length"); }
            return a;
        }, -1);

        this.#data = data;

        this.width = data[0].length;
        this.height = data.length;
    }

    getBit(x: number, y: number): boolean {
        const value = this.#data[y][x];
        if (value == null) { throw new Error(`out of range: ${ x }x${ y }`); }
        return !!value;
    }

    forEach(callback: (x: number, y: number, bitmap: this) => void): void {
        this.#data.forEach((row, y) => {
            row.forEach((bit, x) => {
                if (bit) { callback(x, y, this); }
            });
        });
    }

    get data(): Array<Array<0 | 1>> {
        return this.#data.map((r) => r.slice());
    }

    trim(): Bitmap {
        let data = this.#data.slice();

        while (data.length) {
            const sum = data[0].reduce((a, v) => { return a + v; }, <number>0);
            if (sum) { break; }
            data.shift();
        }

        while (data.length) {
            const sum = data[data.length - 1].reduce((a, v) => { return a + v; }, <number>0);
            if (sum) { break; }
            data.pop();
        }

        while (data[0].length) {
            const sum = data.reduce((a, v) => { return a + v[0]; }, <number>0);
            if (sum) { break; }
            data.forEach((v) => { v.shift(); });
        }

        while (data[0].length) {
            const sum = data.reduce((a, v) => { return a + v[v.length - 1]; }, <number>0);
            if (sum) { break; }
            data.forEach((v) => { v.pop(); });
        }

        return new Bitmap(data);
    }

    static fromData(data: Record<string, string>, bounds?: Box): Bitmap {
        const bbx = Box.fromBbx(data.BBX);
        if (!bounds) { bounds = bbx; }

        const b = bounds;

        const bitmap: Array<Array<0 | 1>> = [ ];
        for (let y = 0; y < bounds.height; y++) {
            bitmap.push(emptyScanline(bounds.width));
        }

        const setBit = (x: number, y: number) => {
            x -= b.x;
            y -= b.y;
            y = (b.height - 1) - y;

            if (bitmap[y] == null || bitmap[y][x] == null) {
                throw new Error(`out of bounds: bounds=${ b.toString() }, x=${ x }, y=${ y }`);
                //console.log("WARNING: drawing out of bounds");
                //return;
            }

            bitmap[y][x] = 1;
        };

        const scanlines = data.bitmap.split(" ").reverse();
        let y = bbx.y;
        for (const _line of scanlines) {
            let x = bbx.x;
            const line = parseInt(_line, 16);
            let mask = 1 << ((_line.length * 4) - 1);

            for (let i = 0; i < bbx.width; i++) {
                if (line & mask) { setBit(x, y); }
                mask >>= 1;
                x++;
            }

            y++;
        }

        return new Bitmap(bitmap);
    }
}

export class Font {
    #data: Map<string, Record<string, string>>

    readonly bounds: Box;

    readonly filename: string;
    readonly name: string;

    constructor(data: Map<string, Record<string, string>>, filename?: string) {
        this.#data = data;

        this.bounds = Box.fromBbx(this.getData("__").FONTBOUNDINGBOX);
        this.filename = filename || "<unknown>";
        const metadata = data.get("__") || { };

        let name = metadata.FAMILY_NAME || metadata.FONT || "<unknown>";
        if (name[0] === '"' && name[name.length - 1] === '"') {
            name = name.substring(1, name.length - 1);
        }
        this.name = name;
    }

    getData(char: string): Record<string, string> {
        const data = this.#data.get(char);
        if (data == null) {
            throw new Error(`no glyph data found: ${ JSON.stringify(char) }`);
        }
        return data;
    }

    getBitmap(char: string): Bitmap {
        return Bitmap.fromData(this.getData(char), this.bounds);
    }

    static fromBdf(content: string, filename?: string): Font {
        let data: Map<string, Record<string, string>> = new Map();

        // Accumulate the header metadata into "__"
        let current: Record<string, string> = { };
        data.set("__", current);

        for (let line of content.split("\n")) {
            line = line.trim();

            const space = line.indexOf(" ");
            if (space === -1) {
                if (skip.indexOf(line) >= 0 || line === "") {
                    // pass
                } else if (line === "BITMAP") {
                    // Initialize a field for the bitmap
                    current.bitmap = "";
                } else if (line.match(/^[0-9A-F]+$/i) && "bitmap" in current) {
                    // Reading a bitmap entry
                    current.bitmap += (current.bitmap ? " ": "") + line;
                } else {
                    throw new Error(`unhandled line: ${ JSON.stringify(line) }`);
                }
                continue;
            }

            // The key-value pair of the line
            const key = line.substring(0, space).trim();
            const value = line.substring(space).trim();

            // Start a new character
            if (key === "STARTCHAR") { current = { }; }

            // Found the character codepoint; set it in the mapping
            if (key === "ENCODING") {
                data.set(String.fromCharCode(parseInt(value)), current);
            }

            // Add the field to the character
            current[key] = value;
        }

        return new Font(data, filename);
    }
}
