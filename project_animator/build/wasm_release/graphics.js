class GraphicsDevice {
    constructor(wasmImportObject, memoryAllocator, openGlContext) {
        if (!wasmImportObject.hasOwnProperty("env")) {
            wasmImportObject.env = {};
        }
        
        this.mem = memoryAllocator;
        this.mem_u8 = memoryAllocator.mem_u8;
        this.mem_buffer = memoryAllocator.wasmMemory.buffer;
        this.u32_max = 4294967295;

        let gl = openGlContext;
        this.gl = gl;

        this.glBuffers = {};
        this.glBufferCounter = 1;

        this.glShaders = {};
        this.glShaderCounter = 1;

        this.glArrayObjects = {};
        this.glArrayObjectCounter = 1;

        this.glTextures = {};
        this.glTextureCounter = 1;

        this.enableStats = false;
        this.indexCount = 0;
        this.drawCount = 0;

        this.boundShaderId = 0;

        this.canUseIntIndices = false;
        const OES_element_index_uint = gl.getExtension("OES_element_index_uint");
        if (OES_element_index_uint) {
            canUseIntIndices = true;
        }

        this.frameBuffers = [];
        this.boundFbo = null;

        let self = this;

        wasmImportObject.env["GfxInitialize"] = function(user_data) {};
        wasmImportObject.env["GfxShutdown"] = function(user_data) {};
        wasmImportObject.env["GfxFinish"] = function() {};

        wasmImportObject.env["GfxEnableStats"] = function(bool_enableds) {
            self.enableStats = bool_enableds;
        };

        wasmImportObject.env["GfxStatsIndexCount"] = function() {
            let result = self.indexCount;
            self.indexCount = 0;
            return result;
        };

        wasmImportObject.env["GfxStatsDrawCount"] = function() {
            let result = self.drawCount;
            self.drawCount = 0;
            return result;
        };
        
        wasmImportObject.env["GfxCreateBuffer"] = function() {
            self.glBufferCounter = (self.glBufferCounter + 1) % (self.u32_max - 1) + 1;
            let bufferId = self.glBufferCounter;

            let startIndex = bufferId;
            while (self.glBuffers.hasOwnProperty(bufferId)) {
                self.glBufferCounter = (self.glBufferCounter + 1) % (self.u32_max - 1) + 1;
                bufferId = self.glBufferCounter;
                
                if (bufferId == startIndex) {
                    console.error("GraphicsDevice.GfxCreateBuffer: ran out of buffer indices");
                    bufferId = 1; // Not 0
                    break;
                }
            }

            self.glBuffers[bufferId] = {
                bufferId: bufferId,
                bufferObject: gl.createBuffer(),
                isIndexBuffer: false,
                indexType: null
            };

            return bufferId;
        };

        wasmImportObject.env["GfxDestroyBuffer"] = function(u32_bufferId) {
            if (!self.glBuffers.hasOwnProperty(u32_bufferId)) {
                console.error("GraphicsDevice.GfxFillArrayBuffer: accessing invalid buffer id(" + u32_bufferId + ")");
            }

            gl.deleteBuffer(self.glBuffers[u32_bufferId].bufferObject);
            delete self.glBuffers[u32_bufferId];
        };

        wasmImportObject.env["GfxFillArrayBuffer"] = function(u32_bufferId, ptr_input, u32_bytes, bool_static) {
            if (!self.glBuffers.hasOwnProperty(u32_bufferId)) {
                console.error("GraphicsDevice.GfxFillArrayBuffer: accessing invalid buffer id(" + u32_bufferId + ")");
            }

            let bufferObject = self.glBuffers[u32_bufferId].bufferObject;
            self.glBuffers[u32_bufferId].isIndexBuffer = false;

           
            gl.bindBuffer(gl.ARRAY_BUFFER, bufferObject);
            gl.bufferData(gl.ARRAY_BUFFER, self.mem_u8, bool_static? gl.STATIC_DRAW : gl.DYNAMIC_DRAW, ptr_input, u32_bytes);
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        };

        wasmImportObject.env["GfxFillIndexBuffer"] = function(u32_bufferId, ptr_input, u32_bytes, u32_bufferType, bool_static) {
            if (!self.glBuffers.hasOwnProperty(u32_bufferId)) {
                console.error("GraphicsDevice.GfxFillIndexBuffer: accessing invalid buffer id(" + u32_bufferId + ")");
            }

            let indexBufferType = null;
            if (u32_bufferType == 1) { // GfxIndexTypeByte
                indexBufferType = gl.UNSIGNED_BYTE;
            }
            else if (u32_bufferType == 2) { // GfxIndexTypeShort
                indexBufferType = gl.UNSIGNED_SHORT;
            }
            else if (u32_bufferType == 3) { // GfxIndexTypeInt
                if (!self.canUseIntIndices) {
                    console.error("GraphicsDevice.GfxFillIndexBuffer: OES_element_index_uint not supported, can't use int indices");
                }
                indexBufferType = gl.UNSIGNED_INT;
            }
            else {
                console.error("GraphicsDevice.GfxFillIndexBuffer: invalid index buffer type. i16 and i32 are supported");
            }

            let bufferObject = self.glBuffers[u32_bufferId].bufferObject;
            self.glBuffers[u32_bufferId].isIndexBuffer = true;
            self.glBuffers[u32_bufferId].indexType = indexBufferType;
            
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, bufferObject);
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, self.mem_u8, bool_static? gl.STATIC_DRAW : gl.DYNAMIC_DRAW, ptr_input, u32_bytes);
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
        };

        wasmImportObject.env["GfxCreateShader"] = function(ptr_vsource, ptr_fsource) {
            let vertexSource = self.mem.PointerToString(ptr_vsource);
            let fragmentSource = self.mem.PointerToString(ptr_fsource);

            let vertexShader = gl.createShader(gl.VERTEX_SHADER);
            gl.shaderSource(vertexShader, vertexSource);
            gl.compileShader(vertexShader);
            let success = gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS);
            if (!success) {
                let message = gl.getShaderInfoLog(vertexShader);
                console.log("GraphicsDevice.GfxCreateShader: error compiling vertex shader: " + message);
                return 0;
            }
    
            let fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
            gl.shaderSource(fragmentShader, fragmentSource);
            gl.compileShader(fragmentShader);
            success = gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS);
            if (!success) {
                let message = gl.getShaderInfoLog(fragmentShader);
                console.log("GraphicsDevice.GfxCreateShader: error compiling fragment shader: " + message);
                return 0;
            }
    
            let shaderProgram = gl.createProgram();
            gl.attachShader(shaderProgram, vertexShader);
            gl.attachShader(shaderProgram, fragmentShader);
            gl.linkProgram(shaderProgram);
            success = gl.getProgramParameter(shaderProgram, gl.LINK_STATUS);
            if (!success) {
                let message = gl.getProgramInfoLog(shaderProgram);
                console.log("GraphicsDevice.GfxCreateShader: error linking shader: " + message);
                return 0;
            }
    
            // Delete shaders
            gl.deleteShader(vertexShader);
            gl.deleteShader(fragmentShader);

            self.glShaderCounter = (self.glShaderCounter + 1) % (self.u32_max - 1) + 1;
            let shaderId = self.glShaderCounter;

            let startIndex = shaderId;
            while (self.glShaders.hasOwnProperty(shaderId)) {
                self.glShaderCounter = (self.glShaderCounter + 1) % (self.u32_max - 1) + 1;
                shaderId = self.glShaderCounter;
                
                if (shaderId == startIndex) {
                    console.error("GraphicsDevice.GfxCreateShader: ran out of shader indices");
                    bufferId = 1; // Not 0
                    break;
                }
            }

            self.glShaders[shaderId] = {
                shaderId: shaderId,
                program: shaderProgram,
                uniforms: [],
                textureUnits: [],
            };
            return shaderId;
        };

        wasmImportObject.env["GfxDestroyShader"] = function(u32_shaderId) {
            if (!self.glShaders.hasOwnProperty(u32_shaderId)) {
                console.error("GraphicsDevice.GfxDestroyShader: accessing invalid shader id(" + u32_shaderId + ")");
            }

            gl.deleteProgram(self.glShaders[u32_shaderId].program);
            delete self.glShaders[u32_shaderId];
        };

        wasmImportObject.env["GfxGetAttributeSlot"] = function(u32_shaderId, ptr_name) {
            if (!self.glShaders.hasOwnProperty(u32_shaderId)) {
                console.error("GraphicsDevice.GfxGetUniformSlot: accessing invalid shader id(" + u32_shaderId + ")");
            }

            let program = self.glShaders[u32_shaderId].program;
            let attribName = self.mem.PointerToString(ptr_name);
            return gl.getAttribLocation(program, attribName);
        }

        wasmImportObject.env["GfxGetUniformSlot"] = function(u32_shaderId, ptr_name) {
            if (!self.glShaders.hasOwnProperty(u32_shaderId)) {
                console.error("GraphicsDevice.GfxGetUniformSlot: accessing invalid shader id(" + u32_shaderId + ")");
            }

            let program = self.glShaders[u32_shaderId].program;
            let uniformName = self.mem.PointerToString(ptr_name);
            let uniforms = self.glShaders[u32_shaderId].uniforms;

            let uniformObject = gl.getUniformLocation(program, uniformName);
            if (uniformObject == null) {
                console.error("GraphicsDevice.GfxGetUniformSlot: invalid uniform: " + uniformName);
            }
            uniformObject.debugName = uniformName;
            
            let uniformIndex = -1;
            for (let i = 0; i < uniforms.length; ++i) {
                if (uniforms[i] == uniformObject) {
                    uniformIndex = i;
                    break;
                }
            }

            if (uniformIndex == -1) {
                uniformIndex = uniforms.length;
                uniforms.push(uniformObject);
            }

            return uniformIndex;
        };

        wasmImportObject.env["GfxCreateShaderVertexLayout"] = function(u32_shaderId) {
            self.glArrayObjectCounter = (self.glArrayObjectCounter + 1) % (self.u32_max - 1) + 1;
            let vaoId = self.glArrayObjectCounter;

            let startIndex = vaoId;
            while (self.glArrayObjects.hasOwnProperty(vaoId)) {
                self.glArrayObjectCounter = (self.glArrayObjectCounter + 1) % (self.u32_max - 1) + 1;
                vaoId = self.glArrayObjectCounter;
                
                if (vaoId == startIndex) {
                    console.error("GraphicsDevice.GfxCreateShaderVertexLayout: ran out of array object indices");
                    vaoId = 1; // Not 0
                    break;
                }
            }

            self.glArrayObjects[vaoId] = {
                arrayObjectId: vaoId,
                arrayObject: gl.createVertexArray(),
                hasIndexBuffer: false,
                indexBuffer: null,
                indexBufferID: 0,
                shaderId: u32_shaderId
            };

            return vaoId;
        };

        wasmImportObject.env["GfxDestroyShaderVertexLayout"] = function(u32_layoutId) {
            if (!self.glArrayObjects.hasOwnProperty(u32_layoutId)) {
                console.error("GraphicsDevice.GfxDestroyShaderVertexLayout: accessing invalid layout id(" + u32_layoutId + ")");
            }

            gl.deleteVertexArray(self.glArrayObjects[u32_layoutId].arrayObject);
            delete self.glArrayObjects[u32_layoutId];
        };

        const GfxAddBufferToLayout = 
        wasmImportObject.env["GfxAddBufferToLayout"] = function(u32_layoutId, attribLocation, u32_bufferId, u32_numComponents, u32_strideBytes, u32_bufferType, u32_dataOffsetBytes) {
            if (!self.glBuffers.hasOwnProperty(u32_bufferId)) {
                console.error("GraphicsDevice.GfxAddBufferToLayout: accessing invalid buffer id(" + u32_bufferId + ")");
            }
            let buffer = self.glBuffers[u32_bufferId].bufferObject;

            if (!self.glArrayObjects.hasOwnProperty(u32_layoutId)) {
                console.error("GraphicsDevice.GfxAddBufferToLayout: accessing invalid layout id(" + u32_layoutId + ")");
            }
            let vao = self.glArrayObjects[u32_layoutId].arrayObject

            let u32_shaderId = self.glArrayObjects[u32_layoutId].shaderId;
            if (!self.glShaders.hasOwnProperty(u32_shaderId)) {
                console.error("GraphicsDevice.GfxAddBufferToLayout: accessing invalid shader id(" + u32_shaderId + ")");
            }
            let program = self.glShaders[u32_shaderId].program;

            const isIndexBuffer = self.glBuffers[u32_bufferId].isIndexBuffer;

            if (isIndexBuffer) {
                self.glArrayObjects[u32_layoutId].hasIndexBuffer = true;
                self.glArrayObjects[u32_layoutId].indexBuffer = buffer;
                self.glArrayObjects[u32_layoutId].indexBufferID = u32_bufferId;
            }

            gl.bindVertexArray(vao);
            if (!isIndexBuffer) {
                let int_type = null;
                if (u32_bufferType == 0) { // GfxBufferTypeFloat32
                    int_type = gl.FLOAT;
                }
                else if (u32_bufferType == 3) { // GfxBufferTypeInt16
                    int_type = gl.SHORT;
                }
                else if (u32_bufferType == 5) { // GfxBufferTypeInt32
                    int_type = gl.INT;
                }
                else {
                    console.error("GraphicsDevice.GfxAddBufferToLayout: invalid buffer type(" + u32_bufferType + ")");
                }

                if (attribLocation >= 0) {
                    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
                    if (u32_bufferType == 3 || u32_bufferType == 5) {
                        gl.vertexAttribIPointer(attribLocation, u32_numComponents, int_type, u32_strideBytes, u32_dataOffsetBytes);
                    }
                    else {
                        gl.vertexAttribPointer(attribLocation, u32_numComponents, int_type, false, u32_strideBytes, u32_dataOffsetBytes);
                    }
                    gl.enableVertexAttribArray(attribLocation);
                    gl.bindBuffer(gl.ARRAY_BUFFER, null); //  probably not needed....
                }
            }
            else {
                if (!self.glBuffers[u32_bufferId].isIndexBuffer) {
                    console.error("GraphicsDevice.GfxAddBufferToLayout: binding non index buffer");
                }
                gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buffer);
            }
            gl.bindVertexArray(null);
        };

        wasmImportObject.env["GfxAddBufferToLayoutByName"] = function(u32_layoutId, ptr_name, u32_bufferId, u32_numComponents, u32_strideBytes, u32_bufferType, u32_dataOffsetBytes) {
            let u32_shaderId = self.glArrayObjects[u32_layoutId].shaderId;
            let program = self.glShaders[u32_shaderId].program;
            let attribName = self.mem.PointerToString(ptr_name);
            let attribLocation = gl.getAttribLocation(program, attribName);

            GfxAddBufferToLayout(u32_layoutId, attribLocation, u32_bufferId, u32_numComponents, u32_strideBytes, u32_bufferType, u32_dataOffsetBytes);
        };

        wasmImportObject.env["GfxWriteToTexture"] = function(u32_textureId, ptr_data, u32_targetFormat, u32_x, u32_y, u32_w, u32_h) {
            let textureFormat = 0;
            let textureDataType = 0;
            let components = 1;
            
            if (u32_targetFormat == 0) { // GfxTextureFormatRGB8
                textureFormat = gl.RGB;
				textureDataType = gl.UNSIGNED_BYTE;
                components = 3;
            }
            else if (u32_targetFormat == 1) { // GfxTextureFormatRGBA8
                textureFormat = gl.RGBA;
				textureDataType = gl.UNSIGNED_BYTE;
                components = 4;
            }
            else if (u32_targetFormat == 2) { // GfxTextureFormatR32F
                textureFormat = gl.RED;
				textureDataType = gl.FLOAT;
                components = 1;
            }
            else if (u32_targetFormat == 3) { // GfxTextureFormatRGB32F
                textureFormat = gl.RGB;
				textureDataType = gl.FLOAT;
                components = 3;
            }
            else if (u32_targetFormat == 4) { // GfxTextureFormatDepth
                textureFormat = gl.DEPTH_COMPONENT;
				textureDataType = gl.FLOAT;
                components = 1;
            }
            else if (u32_targetFormat == 5) { // GfxTextureFormatR8
                textureFormat = gl.RED;
				textureDataType = gl.UNSIGNED_BYTE;
                components = 1;
            }
            else {
                console.error("GraphicsDevice.GfxCreateTexture: invalid target format")
            }
        
            let textureObject = self.glTextures[u32_textureId].textureObject;
            gl.bindTexture(gl.TEXTURE_2D, textureObject); 

            const textureData = new Uint8Array(self.mem_buffer, ptr_data, u32_w * u32_h * components);
            gl.texSubImage2D(gl.TEXTURE_2D, 0, u32_x, u32_y, u32_w, u32_h, textureFormat, textureDataType, textureData);
            gl.bindTexture(gl.TEXTURE_2D, null); 
        };

        wasmImportObject.env["GfxCreateTexture"] = function(ptr_data, u32_width, u32_height, u32_sourceFormat, u32_targetFormat, bool_genMips) {
            if (u32_targetFormat == 4) {
                if (ptr_data != 0) {
                    console.error("GraphicsDevice.GfxCreateTexture: can't provide data for depth texture");
                }
            }
            self.glTextureCounter = (self.glTextureCounter + 1) % (self.u32_max - 1) + 1;
            let textureId = self.glTextureCounter;

            let startIndex = textureId;
            while (self.glTextures.hasOwnProperty(textureId)) {
                self.glTextureCounter = (self.glTextureCounter + 1) % (self.u32_max - 1) + 1;
                textureId = self.glTextureCounter;
                
                if (textureId == startIndex) {
                    console.error("GraphicsDevice.GfxCreateTexture: ran out of texture indices");
                    textureId = 1; // Not 0
                    break;
                }
            }

            let internalFormat = null;
            if (u32_sourceFormat == 0) { // GfxTextureFormatRGB8
                internalFormat = gl.RGB8;
            }
            else if (u32_sourceFormat == 1) { // GfxTextureFormatRGBA8
                internalFormat = gl.RGBA8;
            }
            else if (u32_sourceFormat == 2) { // GfxTextureFormatR32F
                internalFormat = gl.R32F;
            }
            else if (u32_sourceFormat == 3) { // GfxTextureFormatRGB32F
                internalFormat = gl.RGB32F;
            }
            else if (u32_sourceFormat == 4) { // GfxTextureFormatDepth
                internalFormat = gl.DEPTH_COMPONENT32F;
            }
            else if (u32_sourceFormat == 5) { // GfxTextureFormatR8
                internalFormat = gl.R8;
            }
            else {
                console.error("GraphicsDevice.GfxCreateTexture: invalid internal format");
            }

            let textureFormat = null;
            let textureDataType = null;
            if (u32_targetFormat == 0) { // GfxTextureFormatRGB8
                textureFormat = gl.RGB;
				textureDataType = gl.UNSIGNED_BYTE;
            }
            else if (u32_targetFormat == 1) { // GfxTextureFormatRGBA8
                textureFormat = gl.RGBA;
				textureDataType = gl.UNSIGNED_BYTE;
            }
            else if (u32_targetFormat == 2) { // GfxTextureFormatR32F
                textureFormat = gl.RED;
				textureDataType = gl.FLOAT;
            }
            else if (u32_targetFormat == 3) { // GfxTextureFormatRGB32F
                textureFormat = gl.RGB;
				textureDataType = gl.FLOAT;
            }
            else if (u32_targetFormat == 4) { // GfxTextureFormatDepth
                textureFormat = gl.DEPTH_COMPONENT;
				textureDataType = gl.FLOAT;
            }
            else if (u32_targetFormat == 5) { // GfxTextureFormatR8
                textureFormat = gl.RED;
				textureDataType = gl.UNSIGNED_BYTE;
            }
            else {
                console.error("GraphicsDevice.GfxCreateTexture: invalid target format")
            }

            let textureObject = gl.createTexture();
            let isDepth = u32_targetFormat == 4;

            gl.bindTexture(gl.TEXTURE_2D, textureObject); 
            if (isDepth) {
                internalFormat = gl.DEPTH_COMPONENT32F;
                gl.texImage2D(gl.TEXTURE_2D, 0, internalFormat, u32_width, u32_height, 0, textureFormat, textureDataType, null);
            }
            else {
                if (ptr_data == 0) {
                    ptr_data = null;
                }
                gl.texImage2D(gl.TEXTURE_2D, 0, internalFormat, u32_width, u32_height, 0, textureFormat, textureDataType, self.mem_u8, ptr_data);
            }
            if (bool_genMips) {
                gl.generateMipmap(gl.TEXTURE_2D);
            }
            gl.bindTexture(gl.TEXTURE_2D, null); 

            self.glTextures[textureId] = {
                textureId: textureId,
                textureObject: textureObject,
                isDepth: isDepth
            };

            return textureId;
        };

        wasmImportObject.env["GfxDestroyTexture"] = function(u32_textureId) {
            if (!self.glTextures.hasOwnProperty(u32_textureId)) {
                console.error("GraphicsDevice.GfxDestroyTexture: accessing invalid texture id(" + u32_textureId + ")");
            }

            gl.deleteTexture(self.glTextures[u32_textureId].textureObject);
            delete self.glTextures[u32_textureId];
        };

        wasmImportObject.env["GfxSetTextureSampler"] = function(u32_textureId, u32_wrapS, u32_wrapT, u32_min, u32_mip, u32_mag) {
            if (!self.glTextures.hasOwnProperty(u32_textureId)) {
                console.error("GraphicsDevice.GfxSetTextureSampler: accessing invalid texture id(" + u32_textureId + ")");
            }
            
            let textureObject = self.glTextures[u32_textureId].textureObject;

            let min = null;
            if (u32_min == 0) { //GfxFilterNearest
                if (u32_mip == 0) { //GfxFilterNearest
                    min = gl.NEAREST_MIPMAP_NEAREST;
                }
                else if (u32_mip == 1) {// GfxFilterLinear
                    min = gl.NEAREST_MIPMAP_LINEAR;
                }
                else if (u32_mip == 2) { // GfxFilterNone
			        min = gl.NEAREST;
                }
                else {
                    console.error("GraphicsDevice.GfxSetTextureSampler: invalid min / mip combo");
                }
            }
            else if (u32_min == 1) { // GfxFilterLinear
                if (u32_mip == 0) { //GfxFilterNearest
                    min = gl.LINEAR_MIPMAP_NEAREST;
                }
                else if (u32_mip == 1) {// GfxFilterLinear
                    min = gl.LINEAR_MIPMAP_LINEAR;
                }
                else if (u32_mip == 2) { // GfxFilterNone
			        min = gl.LINEAR;
                }
                else {
                    console.error("GraphicsDevice.GfxSetTextureSampler: invalid min / mip combo");
                }
            }
            else {
                console.error("GraphicsDevice.GfxSetTextureSampler: invalid min filter");
            }

            let mag = null;
            if (u32_mag == 0) { //GfxFilterNearest
                mag = gl.NEAREST;
            }
            else if (u32_mag == 1) { // GfxFilterLinear
                mag = gl.LINEAR;
            }
            else {
                console.error("GraphicsDevice.GfxSetTextureSampler: invalid mag filter");
            }

            let wrapS = null;
            if (u32_wrapS == 0) { // GfxWrapRepeat
                wrapS = gl.REPEAT;
            }
            else if (u32_wrapS == 1) { // GfxWrapClamp
                wrapS = gl.CLAMP_TO_EDGE;
            }
            else {
                console.error("GraphicsDevice.GfxSetTextureSampler: invalid wrap s");
            }

            let wrapT = null;
            if (u32_wrapT == 0) { // GfxWrapRepeat
                wrapT = gl.REPEAT;
            }
            else if (u32_wrapT == 1) { // GfxWrapClamp
                wrapT = gl.CLAMP_TO_EDGE;
            }
            else {
                console.error("GraphicsDevice.GfxSetTextureSampler: invalid wrap t");
            }

            gl.bindTexture(gl.TEXTURE_2D, textureObject); 
            gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER, min);
            gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER, mag);
            gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S, wrapS);
            gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T, wrapT);
            gl.bindTexture(gl.TEXTURE_2D, null); 
        };

        const GfxSetTexture = function(u32_shaderId, u32_uniformSlot, u32_textureId) {
            if (!self.glShaders.hasOwnProperty(u32_shaderId)) {
                console.error("GraphicsDevice.GfxSetTexture: accessing invalid shader id(" + u32_shaderId + ")");
            }
            let program = self.glShaders[u32_shaderId].program;
            
            let uniforms = self.glShaders[u32_shaderId].uniforms;
            if (u32_uniformSlot >= uniforms.length) {
                console.error("GraphicsDevice.GfxSetTexture: invalid uniform index");
            }
            let slot = uniforms[u32_uniformSlot]; // slot = getUniformLocation("sampler name");

            if (!self.glTextures.hasOwnProperty(u32_textureId)) {
                console.error("GraphicsDevice.GfxSetTexture: accessing invalid texture id(" + u32_textureId + ")");
            }
            let textureObject = self.glTextures[u32_textureId].textureObject;

            let textureUnits = self.glShaders[u32_shaderId].textureUnits;
            let textureUnit = -1;
            for (let i = 0; i < textureUnits.length; ++i) {
                if (textureUnits[i] == u32_uniformSlot) {
                    textureUnit = i;
                    break;
                }
            }
            if (textureUnit == -1) {
                textureUnit = textureUnits.length;
                textureUnits.push(u32_uniformSlot);
            }

            if (textureUnit > 8) {
                console.error("GraphicsDevice.GfxSetTexture: generated more than 8 samples, intentional?")
            }
            
            self.BindShaderProgramById(u32_shaderId);

            // Activate texture unit first...
            gl.activeTexture(gl.TEXTURE0 + textureUnit);
            // Bind the texture
            gl.bindTexture(gl.TEXTURE_2D, textureObject);
            // Set the "sample name" uniform to Texture unit index
            gl.uniform1i(slot, textureUnit); 
            gl.activeTexture(gl.TEXTURE0);
        };

        wasmImportObject.env["GfxSetUniform"] = function(u32_shaderId, u32_uniformSlot, ptr_data, u32_uniformType, u32_count) {
            if (u32_uniformType == 10) { // GfxUniformTypeTexture
                GfxSetTexture(u32_shaderId, u32_uniformSlot, ptr_data);
                return;
            }

            if (!self.glShaders.hasOwnProperty(u32_shaderId)) {
                console.error("GraphicsDevice.GfxSetUniform: accessing invalid shader id(" + u32_shaderId + ")");
            }

            let program = self.glShaders[u32_shaderId].program;
            
            let uniforms = self.glShaders[u32_shaderId].uniforms;
            if (u32_uniformSlot >= uniforms.length) {
                console.error("GraphicsDevice.GfxSetUniform: invalid uniform index");
            }
            let slot = uniforms[u32_uniformSlot];

            self.BindShaderProgramById(u32_shaderId);

            if (u32_uniformType == 0) { // GfxUniformTypeInt1
                const intData = new Int32Array(self.mem_buffer, ptr_data, u32_count * 1);
                gl.uniform1iv(slot, intData);
            }
            else if (u32_uniformType == 1) { // GfxUniformTypeInt2
                const intData = new Int32Array(self.mem_buffer, ptr_data, u32_count * 2);
                gl.uniform2iv(slot, intData);
            }
            else if (u32_uniformType == 2) { // GfxUniformTypeInt3
                const intData = new Int32Array(self.mem_buffer, ptr_data, u32_count * 3);
                gl.uniform3iv(slot, intData);
            }
            else if (u32_uniformType == 3) { // GfxUniformTypeInt4
                const intData = new Int32Array(self.mem_buffer, ptr_data, u32_count * 4);
                gl.uniform4iv(slot, intData);
            }
            else if (u32_uniformType == 4) { // GfxUniformTypeFloat1
                const floatData = new Float32Array(self.mem_buffer, ptr_data, u32_count * 1);
                gl.uniform1fv(slot, floatData);
            }
            else if (u32_uniformType == 5) { // GfxUniformTypeFloat2
                const floatData = new Float32Array(self.mem_buffer, ptr_data, u32_count * 2);
                gl.uniform2fv(slot, floatData);
            }
            else if (u32_uniformType == 6) { // GfxUniformTypeFloat3
                const floatData = new Float32Array(self.mem_buffer, ptr_data, u32_count * 3);
                gl.uniform3fv(slot, floatData);
            }
            else if (u32_uniformType == 7) { // GfxUniformTypeFloat4
                const floatData = new Float32Array(self.mem_buffer, ptr_data, u32_count * 4);
                gl.uniform4fv(slot, floatData);
            }
            else if (u32_uniformType == 8) { // GfxUniformTypeFloat9
                const floatData = new Float32Array(self.mem_buffer, ptr_data, u32_count * 9);
                gl.uniformMatrix3fv(slot, false, floatData);
            }
            else if (u32_uniformType == 9) { // GfxUniformTypeFloat16
                const floatData = new Float32Array(self.mem_buffer, ptr_data, u32_count * 16);
                gl.uniformMatrix4fv(slot, false, floatData);
            }
            else {
                console.error("GraphicsDevice.GfxSetUniform: invalid uniform type");
            }
        };

        wasmImportObject.env["GfxClearColor"] = function(u32_colorTargetTextureId, u32_depthTargetTextureId, float_r, float_g, float_b) {
            self.BindRenderTarget(u32_colorTargetTextureId, u32_depthTargetTextureId);

            gl.clearColor(float_r, float_g, float_b, 1.0);
            gl.clear(gl.COLOR_BUFFER_BIT);
        };

        wasmImportObject.env["GfxClearDepth"] = function(u32_colorTargetTextureId, u32_depthTargetTextureId, float_depth) {
            self.BindRenderTarget(u32_colorTargetTextureId, u32_depthTargetTextureId);

            gl.clearDepth(float_depth);
            gl.clear(gl.DEPTH_BUFFER_BIT);
        };


        wasmImportObject.env["GfxClearAll"] = function(u32_colorTargetTextureId, u32_depthTargetTextureId, float_r, float_g, float_b, float_depth) {
            self.BindRenderTarget(u32_colorTargetTextureId, u32_depthTargetTextureId);

            gl.clearColor(float_r, float_g, float_b, 1.0);
            gl.clearDepth(float_depth);
            gl.clear(gl.DEPTH_BUFFER_BIT | gl.COLOR_BUFFER_BIT);
        };

        const BlendfuncToEnum = function(u32_func) {
            if (u32_func == 1) { // GfxBlendFuncZero   
                return gl.ZERO;           
            }
            else if (u32_func == 2) { // GfxBlendFuncOne               
                return gl.ONE;
            }
            else if (u32_func == 3) { // GfxBlendFuncSrcColor     
                return gl.SRC_COLOR;     
            }
            else if (u32_func == 4) { // GfxBlendFuncOneMinusSrcColor  
                return gl.ONE_MINUS_SRC_COLOR;
            }
            else if (u32_func == 5) { // GfxBlendFuncDstColor     
                return gl.DST_COLOR;     
            }
            else if (u32_func == 6) { // GfxBlendFuncOneMinusDstColor  
                return gl.ONE_MINUS_DST_COLOR;
            }
            else if (u32_func == 7) { // GfxBlendFuncSrcAlpha          
                return gl.SRC_ALPHA;
            }
            else if (u32_func == 8) { // GfxBlendFuncOneMinusSrcAlpha 
                return gl.ONE_MINUS_SRC_ALPHA; 
            }
            else if (u32_func == 9) { // GfxBlendFuncDstAlpha          
                return gl.DST_ALPHA;
            }
            else if (u32_func == 10) { // GfxBlendFuncOneMinusDstAlpha  
                return gl.ONE_MINUS_DST_ALPHA;
            }
            else if (u32_func == 11) { // GfxBlendFuncConstColor  
                return gl.CONSTANT_COLOR;
            }
            else if (u32_func == 12) { // GfxBlendFuncOneMinusConstColor
                return gl.ONE_MINUS_CONSTANT_COLOR;
            }
            else if (u32_func == 13) { // GfxBlendFuncConstAlpha        
                return gl.CONSTANT_ALPHA;
            }
            else if (u32_func == 14) { // GfxBlendFuncOneMinusconstAlpha
                return gl.ONE_MINUS_CONSTANT_ALPHA;
            }
            else if (u32_func == 15) { // GfxBlendFuncSrcAlphaSaturate  
                return gl.SRC_ALPHA_SATURATE;
            }

            console.error("GraphicsDevice.BlendfuncToEnum: Invalid blend state");
            return null;
        }

        const BlendEqToEnum = function(u32_blendEq) {
            if (u32_blendEq == 0) { // GfxBlendEquationAdd         
                return gl.FUNC_ADD;   
            }
            else if (u32_blendEq == 1) { // GfxBlendEquationSubtract     
                return gl.FUNC_SUBTRACT;  
            }
            else if (u32_blendEq == 2) { // GfxBlendEquationReverseSubtract
                return gl.FUNC_REVERSE_SUBTRACT;
            }
            else if (u32_blendEq == 3) { // GfxBlendEquationMin        
                return gl.MIN;
            }
            else if (u32_blendEq == 4) { // GfxBlendEquationMax   
                return gl.MAX;        
            }

            console.error("GraphicsDevice.BlendEqToEnum: Invalid blend equation");
            return null;
        }

        wasmImportObject.env["GfxSetBlendState"] = function(bool_blend, ptr_optBlendColor, u32_blendDstRgb, 
            u32_blendDstAlpha, u32_blendEquationRgb, u32_blendEquationAlpha, u32_blendSrcRgb, u32_blendSrcAlpha) {
            if (bool_blend) {
                gl.enable(gl.BLEND);
            }
            else {
                gl.disable(gl.BLEND);
            }

            if (ptr_optBlendColor != 0) {
                const floatData = new Float32Array(self.mem_buffer, f32_depthRange, 4);
		        gl.blendColor(floatData[0], floatData[1], floatData[2], floatData[3]);
            }

            let srcAlpha = BlendfuncToEnum(u32_blendSrcAlpha);
            let srcRgb = BlendfuncToEnum(u32_blendSrcRgb);
            let dstAlpha = BlendfuncToEnum(u32_blendDstAlpha);
            let dstRgb = BlendfuncToEnum(u32_blendDstRgb);

            if (u32_blendDstAlpha == u32_blendDstRgb && u32_blendSrcAlpha == u32_blendSrcRgb) { // Same
		        gl.blendFunc(srcRgb, dstRgb);
            }
		    else { // Seperate
                gl.blendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha);
            }

            let alphaEquation = BlendEqToEnum(u32_blendEquationAlpha);
            let rgbEquation = BlendEqToEnum(u32_blendEquationRgb);

            if (u32_blendEquationAlpha == u32_blendEquationRgb) { // Same
                gl.blendEquation(rgbEquation);
            }
            else { // Seperate
                gl.blendEquationSeparate(rgbEquation, alphaEquation);
            }
        };

        wasmImportObject.env["GfxSetCullState"] = function(u32_cullFace, u32_faceWind) {
            if (u32_cullFace == 0) { // GfxCullFaceOff
                gl.disable(gl.CULL_FACE);
            }
            else if (u32_cullFace == 1) { // GfxCullFaceBack
                gl.enable(gl.CULL_FACE);
                gl.cullFace(gl.BACK);
            }
            else if (u32_cullFace == 2) { // GfxCullFaceFront
                gl.enable(gl.CULL_FACE);
                gl.cullFace(gl.FRONT);
            }
            else if (u32_cullFace == 3) { // GfxCullFaceFrontAndBack
                gl.enable(gl.CULL_FACE);
                gl.cullFace(gl.FRONT_AND_BACK);
            }
            else {
                console.error("GraphicsDevice.GfxSetCullState: invalid cull face")
            }

            if (u32_faceWind == 0) { // GfxFaceWindCCW
                gl.frontFace(gl.CCW);
            }
            else if (u32_faceWind == 1) { // GfxFaceWindClockwise
                gl.frontFace(gl.CW);
            }
            else {
                console.error("GraphicsDevice.GfxSetCullState: invalid face winding")
            }
        };

        wasmImportObject.env["GfxSetDepthState"] = function(bool_enable, u32_depthFunc, f32_depthRange) {
            if (bool_enable) {
                gl.enable(gl.DEPTH_TEST);
            }
            else {
                gl.disable(gl.DEPTH_TEST);
            }
            
            let depthFunc = null;
            if (u32_depthFunc == 0) { // GfxDepthFuncAlways
                depthFunc = gl.ALWAYS;
            }
            else if (u32_depthFunc == 1) { // GfxDepthFuncNever
                depthFunc = gl.NEVER;
            }
            else if (u32_depthFunc == 2) { // GfxDepthFuncEqual
                depthFunc = gl.EQUAL;
            }
            else if (u32_depthFunc == 3) { // GfxDepthFuncLEqual
                depthFunc = gl.LEQUAL;
            }
            else if (u32_depthFunc == 4) { // GfxDepthFuncGreater
                depthFunc = gl.GREATER;
            }
            else if (u32_depthFunc == 5) { // GfxDepthFuncGEqual
                depthFunc = gl.GEQUAL;
            }
            else if (u32_depthFunc == 6) { // GfxDepthFuncNotEqual
                depthFunc = gl.NOTEQUAL;
            }
            else if (u32_depthFunc == 7) { // GfxDepthFuncLess
                depthFunc = gl.LESS;
            }
            else {
                console.error("GraphicsDevice.GfxSetDepthState: invalid depth func");
            }
            gl.depthFunc(depthFunc);
        
            if (f32_depthRange != 0) {
                const floatData = new Float32Array(self.mem_buffer, f32_depthRange, 2);
                gl.depthRange(floatData[0], floatData[1]);
            }
        };

        wasmImportObject.env["GfxSetScissorState"] = function(bool_enable, u32_x, u32_y, u32_w, u32_h) {
            if (bool_enable) {
                gl.enable(gl.SCISSOR_TEST);
            }
            else {
                gl.disable(gl.SCISSOR_TEST);
            }
    
            gl.scissor(u32_x, u32_y, u32_w, u32_h);
        };

        wasmImportObject.env["GfxSetWriteMask"] = function(bool_r, bool_g, bool_b, bool_a, bool_depth) {
            gl.colorMask(bool_r, bool_g, bool_b, bool_a);
	        gl.depthMask(bool_depth);
        };

        wasmImportObject.env["GfxSetViewport"] = function(u32_x, u32_y, u32_w, u32_h) {
            gl.viewport(u32_x, u32_y, u32_w, u32_h);
        };

        const DrawModeToEnum = function(u32_mode) {
            if (u32_mode == 0) { // GfxDrawModePoints      
                return gl.POINTS;
            }
            else if (u32_mode == 1) { // GfxDrawModeLines   
                return gl.LINES;     
            }
            else if (u32_mode == 2) { // GfxDrawModeLineStrip    
                return gl.LINE_STRIP;
            }
            else if (u32_mode == 3) { // GfxDrawModeTriangles    
			    return gl.TRIANGLES;
            }
            else if (u32_mode == 4) { // GfxDrawModeTriangleStrip
                return gl.TRIANGLE_STRIP;
            }
            else if (u32_mode == 5) { // GfxDrawModeTriangleFan  
                return gl.TRIANGLE_FAN;
            }
            console.error("GraphicsDevice.DrawModeToEnum: invalid draw mode");
            return null;
        }

        wasmImportObject.env["GfxDraw"] = function(u32_colorTargetTextureId, u32_depthTargetTextureId, 
            u32_vertexLayoutId, u32_drawMode, u32_startIndex, u32_indexCount, u32_instanceCount) {

            self.BindRenderTarget(u32_colorTargetTextureId, u32_depthTargetTextureId);

            if (!self.glArrayObjects.hasOwnProperty(u32_vertexLayoutId)) {
                console.error("GraphicsDevice.GfxDraw: accessing invalid layout id(" + u32_vertexLayoutId + ")");
            }

            let vertexLayout = self.glArrayObjects[u32_vertexLayoutId];
            let vao = vertexLayout.arrayObject;
            let u32_shaderId = vertexLayout.shaderId;

            if (!self.glShaders.hasOwnProperty(u32_shaderId)) {
                console.error("GraphicsDevice.GfxDraw: accessing invalid shader id(" + u32_shaderId + ")");
            }

            let shader = self.glShaders[u32_shaderId];
            self.BindShaderProgramById(u32_shaderId);

            let drawMode = DrawModeToEnum(u32_drawMode);

            gl.bindVertexArray(vao);
            if (vertexLayout.hasIndexBuffer) {
                let indexBufferType = self.glBuffers[vertexLayout.indexBufferID].indexType;
                if (u32_instanceCount <= 1) {
                    gl.drawElements(drawMode, u32_indexCount, indexBufferType, u32_startIndex);
                }
                else {
                    gl.drawElementsInstanced(drawMode, u32_indexCount, indexBufferType, u32_startIndex, u32_instanceCount);
                }
            }
            else { // Not indexed (draw arrays)
                if (u32_instanceCount <= 1) {
                    gl.drawArrays(drawMode, u32_startIndex, u32_indexCount);
                }
                else {
                    gl.drawArraysInstanced(drawMode, u32_startIndex, u32_indexCount, u32_instanceCount);
                }
            }

            if (self.enableStats) {
                self.drawCount += 1;
                self.indexCount += u32_indexCount;
            }

            gl.bindVertexArray(null); 
        };
    }

    BindShaderProgramById(u32_shaderId) {
        const gl = this.gl;

        let newProgram = null;
        if (u32_shaderId != 0) {
            newProgram = this.glShaders[u32_shaderId].program;
        }

        if (u32_shaderId != this.boundShaderId) {
            if (this.boundShaderId != 0) {
                let textureUnits = this.glShaders[this.boundShaderId].textureUnits;
                for (let i = 0; i < textureUnits.length; ++i) {
                    gl.activeTexture(gl.TEXTURE0 + i);
                    gl.bindTexture(gl.TEXTURE_2D, null);
                }
                gl.activeTexture(gl.TEXTURE0);
            }

            gl.useProgram(newProgram);
            this.boundShaderId = u32_shaderId;
        }
    }

    BindRenderTarget(u32_color, u32_depth) {
        const gl = this.gl;

        let fbo = null;
        if (u32_color == 0 && u32_depth == 0) {
            if (this.boundFbo != fbo) {
                gl.bindFramebuffer(gl.FRAMEBUFFER, null);
            }
        }
        else {
            let index = -1;
            for (let i = 0; i < this.frameBuffers.length; ++i) {
                if (this.frameBuffers[i].colorId == u32_color && this.frameBuffers[i].depthId == u32_depth) {
                    index = i; // Found existing FBO to bind
                    break;
                }
            }
            if (index == -1) { // Create new FBO to bind
                fbo = gl.createFramebuffer();
                gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);

                let colorTexture = null;
                let depthTexture = null;

                if (u32_color != 0) {
                    if (!this.glTextures.hasOwnProperty(u32_color)) {
                        console.error("GraphicsDevice.BindRenderTarget: accessing invalid color texture id(" + u32_color + ")");
                    }
                    colorTexture = this.glTextures[u32_color].textureObject

                    gl.bindTexture(gl.TEXTURE_2D, colorTexture);
                    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
                    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
                    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, colorTexture, 0);
                }
                else {
                    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, null, 0);
                }

                if (u32_depth != 0) {
                    if (!this.glTextures.hasOwnProperty(u32_depth)) {
                        console.error("GraphicsDevice.BindRenderTarget: accessing invalid depth texture id(" + u32_depth + ")");
                    }
                    depthTexture = this.glTextures[u32_depth].textureObject

                    gl.bindTexture(gl.TEXTURE_2D, depthTexture);
                    /*if (bool_pcf) {
                        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_COMPARE_MODE, gl.COMPARE_REF_TO_TEXTURE);
                        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_COMPARE_FUNC, gl.LEQUAL);
                    }
                    else {
                        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_COMPARE_MODE, gl.NONE);
                        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_COMPARE_FUNC, gl.LEQUAL);
                    }*/
                    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, depthTexture, 0);
                }
                else {
                    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, null, 0);
                }
                
                gl.bindTexture(gl.TEXTURE_2D, null);

                let fboStatus = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
                if (fboStatus != gl.FRAMEBUFFER_COMPLETE) {
                    let errorMsg = "GraphicsDevice.BindRenderTarget: Incomplete frame buffer: ";

                    if (fboStatus == gl.FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
                        errorMsg += "gl.FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                    }
                    else if (fboStatus == gl.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
                        errorMsg += "gl.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                    }
                    else if (fboStatus == gl.FRAMEBUFFER_INCOMPLETE_DIMENSIONS) {
                        errorMsg += "gl.FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
                    }
                    else if (fboStatus == gl.FRAMEBUFFER_UNSUPPORTED) {
                        errorMsg += "gl.FRAMEBUFFER_UNSUPPORTED";
                    }
                    else if (fboStatus == gl.FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
                        errorMsg += "gl.FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
                    }
                    console.error(errorMsg);
                }

                index = this.frameBuffers.length;
                this.frameBuffers.push({
                    fbo: fbo,
                    colorId: u32_color,
                    depthId: u32_depth,
                    colorTexture: colorTexture,
                    depthTexture: depthTexture,
                });
            }
            else {
                fbo = this.frameBuffers[index].fbo;
                if (this.boundFbo != fbo) {
                    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
                }
            }
        }

        this.boundFbo = fbo;
    }
}