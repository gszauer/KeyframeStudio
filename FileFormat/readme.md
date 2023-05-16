# Keyframe Studio File Format

Each ```.kfs``` file is intended to be read into memory, and can be used directly from the loaded data without any additional parsing. You do need to patch the ```u64``` offsets into pointers. The file contains an identifier string, followed by a header, the hierarchy, any embedded resources, animation, and finally file data (embedded png files). Any time you see ```padding``` in a structure, it was added for alignment and has a value of ```1234```.

## Identifier

All keyframe studio files start with the string literal ```Keyframe Studio```, followed by a null terminator (```\0```).

## Header

The file identifier is followede by an ```80``` byte header. The header is made up of 8 32 bit unsigned integers, followed by 6 64 bit unsigned integers. The header struct looks like this:

```
struct Header {
    u32 nodeCount;
    u32 resourceCount;
    u32 animationCount;
    u32 trackCount;
    u32 nodeUidGenerator;
    u32 resourceUidGenerator;
    u32 animationUidGenerator;
    u32 padding;
    u64 firstNodeOffset;
    u64 firstResourceOffset;
    u64 firstAnimOffset;
    u64 firstTrackOffset;
    u64 firstFrameOffset;
    u64 firstFileOffset;
};
```

The 64 bit unsigned integers are byte offsets from the start of the file. The offsets are stored as 64 bit unsigned integers with the intention that you will patch them into pointers.

## Hierarchy

Following the header, is one or more nodes. Theheader has a ```nodeCount``` variable, which lets you know how many. The Keyframe Studio editor does not visually show the root node, but the root node is saved. Each node has the following structure:

The node stores it's own unique identifier, that of it's parent, first child, and next sibling. This scheme allows for [efficient depth first hierarchy traversal](https://cohost.org/gszauer/post/1103448-hierarchy-representa). The ```nameOffset``` field represents how many bytes from the start of the file you will find the name of the node. The name is a null terminated string. After the name comes various properties of each node.

```
struct Node {
    u32 nodeUid;
    u32 parentUid;
    u32 firstChildUid;
    u32 nextSiblingUid;
    u64 nameOffset; // Offset from start of file
    f32 positionX;
    f32 positionY;
    f32 rotationAngles;
    f32 scaleX;
    f32 scaleY;
    u32 resourceUid;
    f32 tintR;
    f32 tintG;
    f32 tintB;
    f32 tintA;
    u32 visible;
    f32 sourceX;
    f32 sourceY;
    f32 sourceW;
    f32 sourceH;
    f32 pivotX;
    f32 pivotY;
    i32 sortIndex;
};
```

Nodes are saved in depth first order. That is, the parent of each node will be stored before the node its-self. This should allow you to calculate the world transform of each node only once.

## Resources

Following the hierarchy are resources. Each resource is efectivley a file descriptor. Like with nodes, you can get teh first resource with the ```firstResourceOffset``` member of the file header. The loadable resource structure looks like this:

```
struct Resource {
    u32 resourceUid;
    u32 bytes;
    u64 name; // Offset from start of file
    u32 width;
    u32 height;
    u64 data; // Offset from start of file
};
```

The side of each resource is fixed. The ```name``` property is an offset from the start of the file to a null terminated string. Similarly, the ```data``` property is an offset from the start of the file to an embedded png that is ```bytes``` in size.

## Animations

After all the resources are the animations contained in the file. Parsing is done the same way as everything else, the ```name``` property is an offset to a null terminated string, and the ```tracks``` property is an offset to the first track in the animation.

```
struct Animation {
    u32 animUid;
    b32 looping; // 0 or 1
    u32 frameRate;
    u32 frameCount;
    u32 numTracks;
    u32 padding;
    u64 name; // Offset from start of file
    u64 track; // Offset from start of file
};
```

An animation will have multiple tracks. Each track animates one property of a node in the hierarchy. For example, a track might animate the rotation of a node, while another track might animate it's x position. Tracks have the following format:

```
struct Track {
    u32 targetAnimUid;
    u32 targetNodeUid;
    u32 targetProp;
    u32 numFrames;
    u64 frames; // Offset from start of file
    u64 next; // Offset from start of file
};
```

Each track contains two important offsets, along with UID's for what it animates. The ```frames``` offset is the offset of the first frame of the track. Only key frames are recorded, each track keeps track of how many frames it has. The ```next``` offset is the offset of the next track in the animation, or ```0```. The ```targetProp``` represents which property each track is animating, valid values are:

```
enum class TrackType {
	TransformPositionX = 0,
	TransformPositionY = 1,
	TransformRotation = 2,
	TransformScaleX = 3,
	TransformScaleY = 4,
	SpriteTintR = 5,
	SpriteTintG = 6,
	SpriteTintB = 7,
	SpriteVisibility = 8,
	SpriteSourceX = 9,
	SpriteSourceY = 10,
	SpriteSourceW = 11,
	SpriteSourceH = 12,
	NodeSortIndex = 13,
};
```

And finally, each keyframe has the following structure:

```
struct Frame {
    u32 index;
    union {
        f32 fValue;
        u32 uValue;
        i32 iValue;
        b32 bValue;
    };
    u32 padding;
    u32 interpolation;
};
```

All values in the anonymous union are written to the file as an unsigned int (```uValue```) and need to be read in as such. When interpolating an animation, the actual data type of the frame depends on the ```TrackType``` of the track the frame belongs to. A track can be interpolated a few ways, valid interpolation values are:

```
enum class InterpolationType {
	Linear = 0,
	Step = 1,
	EaseIn = 2,
	EaseOut = 3,
	EaseInOut = 4
};
```

## Files

Following all of the frames, is a number of embedded resources. Each resource is going to be an embedded compressed png file. Use the ```Resource``` structs from earlier to load these files.

## Footer

Finally, the ```.kfs``` format ends with the string literal ```Keyframe Studio```, with no trailing null terminator.