/*jshint esversion: 6 */
/*jslint bitwise: true */
/*global WebAssembly */

function MemInjectAllocator(wasmImportObject, wasmMemoryObjectOrMemorySize) {
    let allocatorInstance = {};
    allocatorInstance.wasmMemory = null;
    allocatorInstance.gHeap = 0;
    allocatorInstance.decoder = new TextDecoder();
    allocatorInstance.encoder = new TextEncoder();
    allocatorInstance.wasmInstance = null;
    allocatorInstance.nullChar = allocatorInstance.encoder.encode('\0')[0];

    if (typeof(wasmMemoryObjectOrMemorySize) == 'number') {
        const wasmPageSize = 64 * 1024; // 64 KiB
        const wasmPageCount = Math.ceil(wasmMemoryObjectOrMemorySize / wasmPageSize);
        allocatorInstance.wasmMemory = new WebAssembly.Memory({
            initial: wasmPageCount,
            maximum: wasmPageCount
        });
    }
    else if(wasmMemoryObjectOrMemorySize instanceof WebAssembly.Memory) {
        allocatorInstance.wasmMemory = wasmMemoryObjectOrMemorySize;
    }
    else {
        console.error("InjectGameAllocator: second argument must be WebAssembly.Memory object or a number");
        return null;
    }
    allocatorInstance.u8 = allocatorInstance.mem_u8 = new Uint8Array(allocatorInstance.wasmMemory.buffer, 0, allocatorInstance.wasmMemory.buffer.byteLength);

    if (wasmImportObject === undefined) {
        wasmImportObject = {};    
    }
    if (!wasmImportObject.hasOwnProperty("env")) {
        wasmImportObject.env = {};
    }
    wasmImportObject.env.memory = allocatorInstance.wasmMemory;

    allocatorInstance.PointerToString = function(ptr) {
        let iter = ptr;
        while(allocatorInstance.u8[iter] != 0) {
            iter += 1;

            if (iter - ptr > 5000) {
                console.error("PointerToString loop took too long");
                break;
            }
        }
        return allocatorInstance.decoder.decode(new Uint8Array(this.wasmMemory.buffer, ptr, iter - ptr));
    };

    allocatorInstance.WriteStringToPointer = function(jsString, cPointer) {
        let len = jsString.length;
        const utf8_stirng = allocatorInstance.encoder.encode(jsString);
        for (let i = 0; i < len; ++i) {
            allocatorInstance.u8[cPointer + i] = utf8_stirng[i];
        }
        allocatorInstance.u8[cPointer + len] = allocatorInstance.nullChar;
    }

    allocatorInstance.PrintDebugString = 
    wasmImportObject.env.PrintDebugString = 
    function(ptr) {
        let str = allocatorInstance.PointerToString(ptr);
        console.log(str);
    }

    wasmImportObject.env.memcpy = function(dst, src, bytes) {
        for (let i = 0; i < bytes; ++i) {
            allocatorInstance.u8[dst + i] = allocatorInstance.u8[src + i];
        }
        return dst;
    };

    wasmImportObject.env.memset = function(dst, val, bytes) {
        for (let i = 0; i < bytes; ++i) {
            allocatorInstance.u8[dst + i] = val;
        }
        return dst;
    };

    return allocatorInstance;
}

function MemInitializeHeap(allocatorInstance, wasmInstance) {
    allocatorInstance.wasmInstance = wasmInstance;
    allocatorInstance.heapBase = wasmInstance.exports.MemPlatformAllocate(0);

    const memorySizeBytes = allocatorInstance.wasmMemory.buffer.byteLength;
    let heapSize = memorySizeBytes - allocatorInstance.heapBase;

    let heapPtr = wasmInstance.exports.MemInitializeHeap(allocatorInstance.heapBase, heapSize);

    allocatorInstance.Realloc = function(ptr_data, u32_size) {
        return wasmInstance.exports.MemReallocateOnHeap(heapPtr, ptr_data, u32_size, 0);
    }

    allocatorInstance.Release = function(ptr_data) {
        return  wasmInstance.exports.MemReleaseFromHeap(heapPtr, ptr_data);
    }

    return heapPtr;
}