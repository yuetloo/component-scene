import fs from "fs";
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { Jimp } from "jimp";
import { Font } from "./bdf.js";
const __dirname = dirname(fileURLToPath(import.meta.url));
const base = resolve(__dirname, "../../fonts");
const BLACK = "#000000", WHITE = "#ffffff";
const fonts = [
    {
        filename: "neep-alt-iso8859-1-08x15.bdf",
        size: 15,
        weight: "normal"
    },
    {
        filename: "neep-alt-iso8859-1-08x15-bold.bdf",
        size: 15,
        weight: "bold"
    },
    {
        filename: "neep-alt-iso8859-1-10x20.bdf",
        size: 20,
        weight: "normal"
    },
    {
        filename: "neep-alt-iso8859-1-10x20-bold.bdf",
        size: 20,
        weight: "bold"
    },
    {
        filename: "neep-alt-iso8859-1-12x24.bdf",
        size: 24,
        weight: "normal"
    },
    {
        filename: "neep-alt-iso8859-1-12x24-bold.bdf",
        size: 24,
        weight: "bold"
    },
];
const text0 = "The quick brown fox jumps over the lazy dog.";
const padding = [50, 25, 25, 25];
export async function createFontPreview(font, text, isDark) {
    const { width, height } = font.bounds;
    const c = isDark ? 0xffffff : 0;
    const preview = new Jimp({
        width: (width + 1) * 48 + padding[1] + padding[3],
        height: (height + 1) * 4 + padding[0] + padding[2],
        color: (isDark ? BLACK : WHITE)
    });
    const setPixel = (x, y) => {
        const offset = (padding[0] + y) * preview.bitmap.width + (padding[1] + x);
        preview.bitmap.data[4 * offset] = (c >> 16) & 0xff;
        preview.bitmap.data[4 * offset + 1] = (c >> 8) & 0xff;
        preview.bitmap.data[4 * offset + 2] = (c >> 0) & 0xff;
    };
    const putChar = (chr, bx, by) => {
        const bitmap = font.getBitmap(chr);
        bitmap.forEach((x, y) => { setPixel(bx + x, by + y); });
    };
    //{
    //    const font = await Jimp.loadFont(isDark ? Jimp.FONT_SANS_16_WHITE: Jimp.FONT_SANS_16_BLACK);
    //    preview.print({ font, x: padding[3], y: 20, text });
    //}
    for (let i = 33; i < 127; i++) {
        const chr = String.fromCharCode(i);
        const index = i - 33;
        const bx = (index % 48) * (width + 1);
        const by = Math.trunc(index / 48) * (height + 1);
        putChar(chr, bx, by);
    }
    for (let i = 0; i < text0.length; i++) {
        putChar(text0[i], i * (width + 1), 3 * (height + 1));
    }
    return preview;
}
async function createPreview(isDark) {
    const previews = [];
    for (const { filename, size, weight } of fonts) {
        const path = resolve(base, filename);
        const font = Font.fromBdf(fs.readFileSync(path).toString(), filename);
        console.log({ font, size, weight });
        const text = `font-${size}pt-${weight}`;
        previews.push(await createFontPreview(font, text, isDark));
    }
    {
        const maxWidth = previews.reduce((a, img) => {
            return Math.max(img.bitmap.width, a);
        }, 0);
        const sumHeight = previews.reduce((a, img) => {
            return img.bitmap.height + a;
        }, 0);
        const preview = new Jimp({
            width: maxWidth,
            height: sumHeight,
            color: (isDark ? BLACK : WHITE)
        });
        let y = 0;
        for (const img of previews) {
            preview.composite(img, 0, y);
            y += img.bitmap.height;
        }
        return preview;
    }
}
(async function () {
    for (const isDark of [false, true]) {
        const preview = await createPreview(isDark);
        const path = resolve(base, `preview-${isDark ? "dark" : "light"}.png`);
        preview.write(path); // THIS IS DUMB; can it be fixed? Jimp enforces an extension in TS-world
    }
})();
//# sourceMappingURL=generate.js.map