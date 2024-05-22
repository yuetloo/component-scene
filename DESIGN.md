Scene Graph Design
==================

A **Scene Graph** is organized as a collection of *Nodes**
connected 

**Scene properties:**

- `nextFree` - the head of the FreeLinkedList
- `root` - the root **Group Node*
- `tick` - the global scene timestamp
- `renderHead` - the head of the RenderLinkedList, where rendering begins from when `ffx_scene_render` is performed
- `renderTail` - the tail of the RenderLinkedList, where new **Render Nodes** are appended

**Node properties**

The node properties are highly overloaded depending on the type of
**Node**, but this outlines the general usage of each:

- `nextNode` - 
  - FreeNode: pointer to next FreeNode in the FreeLinkedList
  - SequenceNode: pointer to next sibling SequenceNode
  - RenderNode: pointer to the next RenderNode in the RenderLinkedList
  - AnimateNode: pointer to the AnimatePropertyNode for the animation
  - AnimatePropertyNode: pointer to the next AnimateNode (NULL if tail)
- `animate` - 
  - SequenceNode: the head of the AnimateNode linked list for the node animations
  - AnimateProperyNode: the `onComplete` (if any) for the animation to call when complere
- `func`
  - SequenceNode: the sequence function that adds **Render Nodes** to the render list
  - render nodes: the render function that renders the node to the buffer
  - animate nodes: the function to perform interpolation between the start and end values of the animation
  - animate-property nodes: the animation curve for the property
- `pos`
  - SequenceNode: the relative position with respect to the parent
  - RenderNode: the world position of the node (rendering is flat, non-hierarchal)
- `a`
  - SequenceNode: node-specidific property
  - GroupNode: pointer to first child
  - RenderNode: usually a snapshot of the related sequence node `.a` when sequenced
  - AnimateNode: the animation duration
  - AnimateProperyNode: the start value of the property to interpolate
- `b`
  - SequenceNode: node-specidific property
  - GroupNode: pointer to last child
  - RenderNode: usually a snapshot of the related sequence node `.b` when sequenced
  - AnimateNode: the animation end time
  - AnimateProperyNode: the end value of the property to interpolate


## Node Types


### FreeNode

A FreeNode is an unallocated Node which can be used for any Node
type, stored in the FreeLinkedList.

When the **Scene** is initialized all the Nodes are linked
together and the `Scene.freeNode` points to the head.


### SequenceNode

Each **Sequence Node** represents a node in the Scene Graph.

Some may represent a renderable node (such as an image or box)
while others may merely represent relationships via a group
node.

When a sequence node is sequenced (via `ffx_scene_sequence`)
the node may add 0 or more **Render Nodes*.


### RenderNode

A RenderNode represents an action which is required during
the `ffx_scene_renderFragment` and is is called for each
fragment to render the viewport.

The RenderLinkedList is completely recycled at the beginning
of `ffx_scene_sequence`, and the SequenceNode `.func` is
responsible for adding any RenderNode(s) necessary.

Since the RenderLinkedList is flat (i.e. non-hierarchal) all
positions are automatically adjusted to world coordinates
and the sequence function should take a snapshot of the
current values, as updates outside a sequence must not
affect rendering.

Some sequencing and rendering functions may adjust the
RenderNode `.pos`, `.a` and `.b` to optimize and cache
calculations.

For example, the BoxNode creates its RenderNode clipped to
the display, so the RenderNode is unaware of the actual
position and size of the cooresponding SequenceNode.


### AnimationNode

An AnimationNode is paired with a AnimationPropertyNode which
contains all the necessary properties required to animate a
node.

The AnimationNode includes the timing information for the
animation as well as the animation function used to interpolate
between the start and end values.

The AnimationPropertyNode includes the start and end values
for the property being animated as well as the curve function
used to interpolate the parametric t.

Animations are added to the head of a SequenceNode
AnimationLinkedList. When an animation is stopped, the head of
the SequenceNode's AnimationLinkedList is stopped, which will
get picked up on the next sequence. Any additional animations
added will be placed in front of the stopped animation and
therefore unaffected by the pending stop.


### AnimationPropertyNode

See the AnimationNode for more details.


## Notes

### Removing nodes

A SequenceNode is not immediately removed, instead the `.func`
is set to a special sequence function that is never called; it
is intercepted during graph traversal.

During traversal the node is reclaimated and moved to the
FreeLinkedList, allowing proper clean-up.
