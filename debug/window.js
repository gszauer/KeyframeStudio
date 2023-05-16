
class GameWindow {
    constructor(wasmImportObject, allocator, canvasName) {
        if (!wasmImportObject.hasOwnProperty("env")) {
            wasmImportObject.env = {};
        }

        this.dpi = window.devicePixelRatio || 1;
        this.mem = allocator;
        this.wasmInstance = null;
        this.userDataPtr = 0;
        this.canvas = document.getElementById(canvasName);
        this.gl = this.canvas.getContext('webgl2');
        this.running = false;
        this.queue = [];

        let boundingRect = this.canvas.getBoundingClientRect();
        this.lastDisplayWidth = boundingRect.width;
        this.lastDisplayHeight = boundingRect.height;
        this.lastBufferWidth = this.canvas.width;
        this.lastBufferHeight = this.canvas.height;

        this.currentButtonState = new Uint32Array(2);
        this.previousButtonState = new Uint32Array(2);
        this.currentButtonState[0] = this.currentButtonState[1] = 0;
        this.previousButtonState[0] = this.previousButtonState[1] = 0;

        this.mouseX = 0;
        this.mouseY = 0;
        this.mouseScroll = 0;

        this.prevX = 0;
        this.prevY = 0;
        this.prevScroll = 0;

        this.clipboard = "";
        this.clipboard_ptr = 0;
        this.clipboard_len = 0;

        this.maxNumTouches = 5;
        this.numTouches = 0;
        this.prevNumTouches = 0;
        this.touches = [ 
            { x: 0, y: 0, id: null }, { x: 0, y: 0, id: null }, 
            { x: 0, y: 0, id: null }, { x: 0, y: 0, id: null },
            { x: 0, y: 0, id: null }
         ];
        this.prevTouches = [ 
            { x: 0, y: 0, id: null }, { x: 0, y: 0, id: null },
            { x: 0, y: 0, id: null }, { x: 0, y: 0, id: null },
            { x: 0, y: 0, id: null }
         ];

        let self = this;

        this.canvas.addEventListener('touchstart', (event) => {
            let devicePixelRatio = window.devicePixelRatio || 1;
            if (event.defaultPrevented) {
                return false;
            }

            self.numTouches = event.touches.length;
           
            for (let i = 0; i < self.numTouches; ++i) {
                let found = false;
                for (let j = 0; j < self.maxNumTouches; ++j) {
                    if (self.touches[j].id == event.touches[i].identifier) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    for (let j = 0; j < self.maxNumTouches; ++j) {
                        if (self.touches[j].id == null) {
                            self.touches[j].id = event.touches[i].identifier;
                            self.touches[j].x = (event.touches[i].clientX - self.canvas.offsetLeft) * devicePixelRatio;
                            self.touches[j].y = (event.touches[i].clientY - self.canvas.offsetTop) * devicePixelRatio;
                            break;
                        }
                    }
                }
            }

            event.preventDefault();
            return false;
        }, false);

        this.canvas.addEventListener('touchmove', (event) => {
            let devicePixelRatio = window.devicePixelRatio || 1;
            if (event.defaultPrevented) {
                return false;
            }

            self.numTouches = event.touches.length;
            let updated = 0;
            for (let i = 0; i < self.numTouches; ++i) { // Find and update each touch
                for (let j = 0; j < self.maxNumTouches; ++j) {
                    if (self.touches[j].id == event.touches[i].identifier) {
                        self.touches[j].x = (event.touches[i].clientX - self.canvas.offsetLeft) * devicePixelRatio;
                        self.touches[j].y = (event.touches[i].clientY - self.canvas.offsetTop) * devicePixelRatio;
                        updated += 1;
                        break;
                    }
                }
            }

            if (updated != self.numTouches) {
                console.error("Mismatched number of touches");
            }

            event.preventDefault();
            return false;
        }, false);

        const TouchEndHandler = function(event) {
            self.numTouches = event.touches.length;
            for (let i = 0; i < self.maxNumTouches; ++i) {
                if (self.touches[i].id == null) {
                    continue;
                }
                let found = false;
                for (let j = 0; j < self.numTouches; ++j) {
                    if (self.touches[i].id == event.touches[j].identifier) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    self.touches[i].id = null;
                }
            }

            event.preventDefault();
            return false;
        };

        this.canvas.addEventListener('touchend', TouchEndHandler, false);

          this.canvas.addEventListener('touchcancel', TouchEndHandler, true);


        const KeyMap = {
            'Backspace': 3, 'Enter': 5, 'Shift': 6, 'ShiftLeft': 6, 'ShiftRight': 6, 
            'Control': 7, 'ControlLeft': 7, 'ControlRight': 7, 'Alt': 8, 'AltRight': 8, 
            'AltLeft': 8, 'CapsLock': 9, 'Escape': 10, 'Space': 11, 'ArrowLeft': 12, 
            'ArrowUp': 13, 'ArrowRight': 14, 'ArrowDown': 15, 'Delete': 16, 'Digit0': 17, 
            'Digit1': 18, 'Digit2': 19, 'Digit3': 20, 'Digit4': 21, 'Digit5': 22, 
            'Digit6': 23, 'Digit7': 24, 'Digit8': 25, 'Digit9': 26, 'KeyA': 27, 
            'KeyB': 28, 'KeyC': 29, 'KeyD': 30, 'KeyE': 31, 'KeyF': 32, 'KeyG': 33, 
            'KeyH': 34, 'KeyI': 35, 'KeyJ': 36, 'KeyK': 37, 'KeyL': 38, 'KeyM': 39, 
            'KeyN': 40, 'KeyO': 41, 'KeyP': 42, 'KeyQ': 43, 'KeyR': 44, 'KeyS': 45, 
            'KeyT': 46, 'KeyU': 47, 'KeyV': 48, 'KeyW': 49, 'KeyX': 50, 'KeyY': 51, 
            'KeyZ': 52, 'Semicolon': 53, 'Equal': 54, 'Comma': 55, 'Minus': 56, 
            'Period': 57, 'Slash': 58, 'Backquote': 59, 'BracketLeft': 60, 
            'Backslash': 61, 'BracketRight': 62, 'Quote': 63, 'Tab': 64
        };

        const MouseMotionHandler = (event) => {
            let devicePixelRatio = window.devicePixelRatio || 1;
            self.mouseX = (event.pageX - self.canvas.offsetLeft) * devicePixelRatio;
            self.mouseY = (event.pageY - self.canvas.offsetTop) * devicePixelRatio;
        };

        this.canvas.addEventListener("mousemove",  MouseMotionHandler, true);
        this.canvas.addEventListener("mouseenter", MouseMotionHandler, true);
        this.canvas.addEventListener("mouseleave", MouseMotionHandler, true);
        this.canvas.addEventListener('wheel', (event) => {
            if (event.defaultPrevented) {
                return false;
            }

            if (event.deltaY < 0.0) {
                this.mouseScroll = 1;
            }
            else if (event.deltaY > 0.0) {
                this.mouseScroll = -1;
            }
            else {
                this.mouseScroll = 0;
            }

            event.preventDefault();
            return false;
        }, false);

        this.canvas.addEventListener('mousedown', (event) => {
            if (event.defaultPrevented) {
                return false;
            }

            if (event.button == 0) { // main / left
                self.currentButtonState[0] |= (1 << 1); // KeyboardCodeLeftMouse: 1
            }
            else if (event.button == 1) { // aux / middle
                self.currentButtonState[0] |= (1 << 4); // KeyboardCodeMiddleMouse: 4
            }
            else if (event.button == 2) { // secondary / right
                self.currentButtonState[0] |= (1 << 2); // KeyboardCodeRightMouse: 2
            }

            event.preventDefault();
            return false;
        }, false);

        this.canvas.addEventListener('mouseup', (event) => {
            if (event.defaultPrevented) {
                return false;
            }

            if (event.button == 0) { // main / left
                self.currentButtonState[0] &= ~(1 << 1); // KeyboardCodeLeftMouse: 1
            }
            else if (event.button == 1) { // aux / middle
                self.currentButtonState[0] &= ~(1 << 4); // KeyboardCodeMiddleMouse: 4
            }
            else if (event.button == 2) { // secondary / right
                self.currentButtonState[0] &= ~(1 << 2); // KeyboardCodeRightMouse: 2
            }

            event.preventDefault();
            return false;
        }, false);

        // Do we want window wide key focus? or just canvas wide?
        window.addEventListener("keydown", (event) => {
            if (event.defaultPrevented) {
                return false; // Do nothing if the event was already processed
            }

            if (KeyMap.hasOwnProperty(event.code)) {
                const code = KeyMap[event.code];
                const index = ~~(code / 32);
                const bit = ~~(code % 32);

                self.currentButtonState[index] |= (1 << bit);
                
                self.queue.push(code);
            }

            // Cancel the default action to avoid it being handled twice
            event.preventDefault();
            return false;
        }, false);

        window.addEventListener("keyup", (event) => {
            if (event.defaultPrevented) {
                return false; // Do nothing if the event was already processed
            }

            if (KeyMap.hasOwnProperty(event.code)) {
                const code = KeyMap[event.code];
                const index = ~~(code / 32);
                const bit = ~~(code % 32);

                self.currentButtonState[index] &= ~(1 << bit);
            }

            // Cancel the default action to avoid it being handled twice
            event.preventDefault();
            return false;
        }, false);

        wasmImportObject.env["WindowUpdateTitle"] = function(titlePtr) {
            window.document.title = self.mem.PointerToString(titlePtr);
        }

        wasmImportObject.env["PushKey"] = function(u32_scanCode) {
            self.queue.push(u32_scanCode);
        }

        wasmImportObject.env["ConsumeKeyQueue"] = function() {
            if (self.queue.length == 0) {
                return 1; // KeyboardCodeLeftMouse
            }

            let key = self.queue[0];
            self.queue.shift();
            return key;
        }

        wasmImportObject.env["ClearKeyQueue"] = function() {
            self.queue.length = 0;
        }

        wasmImportObject.env["WriteClipboard"] = function(ptr_string) {
            if (ptr_string == 0) {
                self.clipboard = "";
            }
            else {
                self.clipboard = self.mem.PointerToString(ptr_string);
            }
        }

        wasmImportObject.env["WindowOpenURL"] = function(ptr_string) {
            if (ptr_string != 0) {
                let urlString = self.mem.PointerToString(ptr_string);
                window.open(urlString, '_blank');
            }
        }

        wasmImportObject.env["ReadClipboard"] = function() {
            if (self.clipboard.length == 0) {
                return 0;
            }

            if (self.clipboard_len < self.clipboard.length) {
                self.clipboard_ptr = self.mem.Realloc(self.clipboard_ptr, self.clipboard.length + 1);
                self.clipboard_len = self.clipboard.length;
                self.mem.WriteStringToPointer(self.clipboard, self.clipboard_ptr);
            }

            return self.clipboard_ptr;
        }

        wasmImportObject.env["KeyboardDown"] = function(u32_scanCode) {
            const index = ~~(u32_scanCode / 32);
            const bit = ~~(u32_scanCode % 32);

            const result = self.currentButtonState[index] & (1 << bit)
            return result != 0;
        }

        wasmImportObject.env["KeyboardPrevDown"] = function(u32_scanCode) {
            const index = ~~(u32_scanCode / 32);
            const bit = ~~(u32_scanCode % 32);

            const result = self.previousButtonState[index] & (1 << bit)
            return result != 0;
        }
        
        wasmImportObject.env["MouseDown"] = function(u32_scanCode) {
            const index = ~~(u32_scanCode / 32);
            const bit = ~~(u32_scanCode % 32);

            const result = self.currentButtonState[index] & (1 << bit)
            return result != 0;
        }

        wasmImportObject.env["MousePrevDown"] = function(u32_scanCode) {
            const index = ~~(u32_scanCode / 32);
            const bit = ~~(u32_scanCode % 32);

            const result = self.previousButtonState[index] & (1 << bit)
            return result != 0;
        }

        wasmImportObject.env["MouseGetX"] = function() {
            return self.mouseX;
        };

        wasmImportObject.env["MouseGetY"] = function() {
            return self.mouseY;
        };

        wasmImportObject.env["MouseGetScroll"] = function() {
            return self.mouseScroll;
        };

        wasmImportObject.env["MousePrevX"] = function() {
            return self.prevX;
        };

        wasmImportObject.env["MousePrevY"] = function() {
            return self.prevY;
        };

        wasmImportObject.env["MousePrevScroll"] = function() {
            return self.prevScroll;
        };

        wasmImportObject.env["TouchGetMaxContacts"] = function() {
            return self.maxNumTouches;
        };

        wasmImportObject.env["TouchGetX"] = function(u32_touchIndex) {
            if (u32_touchIndex >= self.maxNumTouches) {
                console.error("Touch index out of bounds: " + u32_touchIndex + " / " + self.maxNumTouches);
            }
            return self.touches[u32_touchIndex].x;
        }

        wasmImportObject.env["TouchGetY"] = function(u32_touchIndex) {
            if (u32_touchIndex >= self.maxNumTouches) {
                console.error("Touch index out of bounds: " + u32_touchIndex + " / " + self.maxNumTouches);
            }
            return self.touches[u32_touchIndex].y;
        }

        wasmImportObject.env["TouchGetPrevX"] = function(u32_touchIndex) {
            if (u32_touchIndex >= self.maxNumTouches) {
                console.error("Touch index out of bounds: " + u32_touchIndex + " / " + self.maxNumTouches);
            }
            return self.prevTouches[u32_touchIndex].x;
        }
        wasmImportObject.env["TouchGetPrevY"] = function(u32_touchIndex) {
            if (u32_touchIndex >= self.maxNumTouches) {
                console.error("Touch index out of bounds: " + u32_touchIndex + " / " + self.maxNumTouches);
            }
            return self.prevTouches[u32_touchIndex].y;
        }

        wasmImportObject.env["TouchIsActive"] = function(u32_touchIndex) {
            if (u32_touchIndex >= self.maxNumTouches) {
                console.error("Touch index out of bounds: " + u32_touchIndex + " / " + self.maxNumTouches);
            }
            return self.touches[u32_touchIndex].id != null;
        }

        wasmImportObject.env["TouchWasActive"] = function(u32_touchIndex) {
            if (u32_touchIndex >= self.maxNumTouches) {
                console.error("Touch index out of bounds: " + u32_touchIndex + " / " + self.maxNumTouches);
            }
            return self.prevTouches[u32_touchIndex].id != null;
        }
    }

    AttachToWasmInstance(wasmInstance) {
        let useRequestAnimFrames = false;

        window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
            if (key=="fast") {
                useRequestAnimFrames = true;
            }
        });

        this.wasmInstance = wasmInstance;
        const exports = wasmInstance.exports;
        this.userDataPtr = null;
        this.running = true;

        let lastTime = 0;
        let deltaTime = 0.0;
        let self = this;
       
        const GameWindowUpdate = function(timestamp) {
            if (!useRequestAnimFrames) {
                timestamp = performance.now();
            }
            deltaTime = (timestamp - lastTime) / 1000.0;
            lastTime = timestamp;
            //console.log("delta time: " + deltaTime);

            let boundingRect = self.canvas.getBoundingClientRect();
            const windowHeight = (window.innerHeight || document.documentElement.clientHeight);
            const windowWidth = (window.innerWidth || document.documentElement.clientWidth)

            let vVis = boundingRect.bottom >= 0  && boundingRect.top <= windowHeight;
            let hVis = boundingRect.right >= 0 && boundingRect.left <= windowWidth;
            let visible = hVis && vVis;

            let expectedDisplayWidth = windowWidth;//boundingRect.width;
            let expectedDisplayHeight = windowHeight;//boundingRect.height;
            let expectedBufferWidth = Math.floor(expectedDisplayWidth * self.dpi);
            let expectedBufferHeight = Math.floor(expectedDisplayHeight * self.dpi);

            if (expectedDisplayWidth != self.lastDisplayWidth || expectedDisplayHeight != self.lastDisplayHeight || 
            expectedBufferWidth != self.lastBufferWidth || expectedBufferHeight != self.lastBufferHeight) {
                //console.log("Resizing canvas, Display(" + expectedDisplayWidth + ", " + expectedDisplayHeight + "), Buffer(" + expectedBufferWidth + ", " + expectedBufferHeight + ")");

                self.canvas.style.width = expectedDisplayWidth + "px";
                self.canvas.style.height = expectedDisplayHeight + "px";
                self.canvas.width = expectedBufferWidth;
                self.canvas.height = expectedBufferHeight;

                self.lastDisplayWidth = expectedDisplayWidth;
                self.lastDisplayHeight = expectedDisplayHeight;
                self.lastBufferWidth = expectedBufferWidth;
                self.lastBufferHeight = expectedBufferHeight;
            }

            if (self.running) {
                exports.Update(deltaTime, self.userDataPtr);
                if (visible) {
                    exports.Render(0, 0, expectedBufferWidth, expectedBufferHeight, self.dpi, self.userDataPtr);
                }

                self.previousButtonState[0] = self.currentButtonState[0];
                self.previousButtonState[1] = self.currentButtonState[1];
                self.prevX = self.mouseX;
                self.prevY = self.mouseY;
                self.prevScroll = self.mouseScroll;
                self.mouseScroll = 0;
                self.prevNumTouches = self.numTouches;
                for (let i = 0; i < self.maxNumTouches; ++i) {
                    self.prevTouches[i]. x = self.touches[i].x;
                    self.prevTouches[i].y  = self.touches[i].y;
                    self.prevTouches[i].id = self.touches[i].id;
                }
            }

            if (self.running) {
                if (useRequestAnimFrames) {
                    window.requestAnimationFrame(GameWindowUpdate);
                }
            }
        }

        if (useRequestAnimFrames) {
            console.log("Update driven by request animation farme");
            this.userDataPtr = exports.Initialize();

            lastTime = performance.now();
            window.requestAnimationFrame(GameWindowUpdate);
        }
        else {
            console.log("Update driven by set interval (14 ms)");
            this.userDataPtr = exports.Initialize();

            lastTime = performance.now();
            window.setInterval(GameWindowUpdate, 14, 0); 
        }
    }

    DestroyWindow() {
        this.running = false;
        if (this.userDataPtr != null) {
            if (this.clipboard_ptr != 0) {
                this.mem.Release(this.clipboard_ptr);
                this.clipboard_ptr = 0;
                this.clipboard_len = 0;
            }
            this.wasmInstance.exports.Shutdown(this.userDataPtr);
        }
    }
}