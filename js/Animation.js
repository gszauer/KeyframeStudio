
export class Track {
    frames = null;
    clip = null; // Owning clip
    target = null; // Sprite or transform, depends on type
    type = null; // string, ie: position.x

    get duration() {
        return this.clip.duration;
    }

    constructor(clip, target, type) {
        this.frames = [];
        this.clip = clip;
        this.target = target;
        this.type = type;
    }
    
    AddFrame(time, value) {
        const frames = this.frames;
        const length = this.frames.length;

        const frame = {
            time: time,
            value: value
        };

        if (length == 0) {
            frames.push(frame);
        }
        else if (time < frames[0].time) {
            frames.splice(0, 0, frame);
        }
        else if (time > frames[length - 1].time) {
            frames.push(frame);
        }
        else {
            for (let i = 0; i < length; ++i) {
                if (frames[i].time > time) {
                    frames.splice(i, 0, frame);
                    break;
                }
            }
        }

        return frame;
    }

    Evaluate(time) {

    }
}

export default class Clip {
    name = null;
    duration = 0;
    tracks = null;

    static TrackPositionX = "position.x";
    static TrackPositionY = "position.y";
    static TrackRotation = "rotation";
    static TrackScaleX = "scale.x";
    static TrackScaleY = "scale.y";
    static VisibleTrack = "visible";
    static TintTrack = "tint";

    constructor(name, duration) {
        if (!name) {
            name = "Animation Clip";
        }
        if (!duration) {
            duration = 0;
        }

        this.name = "" + name;
        this.duration = duration;
        this.tracks = [];
    }

    TrackIndex(target, track) {
        const tracks = this.tracks;
        const length = this.tracks.length;
        for (let i = 0; i < length; ++i) {
            if (tracks[i].target === target) {
                if (tracks[i].type === track) {
                    return i;
                }
            }
        }

        return -1;
    }

    AddTrack(target, track) {    
        let trackIndex = this.TrackIndex(target, track);
        if (trackIndex === -1) {
            const result = new Track(this, target, track);
            this.tracks.push(result);
            trackIndex = this.tracks.length - 1;
        }
        
        return this.tracks[trackIndex];
    }

    RemoveTrack(target, track) {
        const toRemove = this.TrackIndex(target, track);
        if (toRemove === -1) {
            return;
        }

        this.tracks.splice(toRemove, 1);
    }

    Evaluate(time) {
        time = time % this.duration;

        const tracks = this.tracks;
        const length = this.tracks.length;
        for (let i = 0; i < length; ++i) {
            tracks[i].Evaluate(time);
        }
    }
}