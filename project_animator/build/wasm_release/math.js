class GameMath {
    constructor(wasmImportObject) {
        if (!wasmImportObject.hasOwnProperty("env")) {
            wasmImportObject.env = {};
        }
        let self = this;

        wasmImportObject.env["MathSin"] = function(x) {
            return Math.sin(x);
        };

        wasmImportObject.env["MathCos"] = function(x) {
            return Math.cos(x);
        };

        wasmImportObject.env["MathTan"] = function(x) {
            return Math.tan(x);
        };

        wasmImportObject.env["MathSqrt"] = function(x) {
            return Math.sqrt(x);
        };

        wasmImportObject.env["MathAtan2"] = function(y, x) {
            return Math.atan2(y, x);
        };

        wasmImportObject.env["MathRandom"] = function() {
            return Math.random();
        };

        wasmImportObject.env["MathExp"] = function(x) {
            return Math.exp(x)
        };

        wasmImportObject.env["MathLog"] = function(x) {
            return Math.log(x)
        };

        wasmImportObject.env["MathPow"] = function(x, y) {
            return Math.pow(x, y)
        };

        wasmImportObject.env["MathFloor"] = function(x) {
            return Math.floor(x)
        };

        wasmImportObject.env["MathCeil"] = function(x) {
            return Math.ceil(x)
        };

        wasmImportObject.env["MathRound"] = function(x) {
            return Math.round(x)
        };

        wasmImportObject.env["MathACos"] = function(a) {
            return Math.acos(a);
        }

        // https://blog.codefrau.net/2014/08/deconstructing-floats-frexp-and-ldexp.html
        const frexp = function(value) {
            if (value === 0) return [value, 0];
            var data = new DataView(new ArrayBuffer(8));
            data.setFloat64(0, value);
            var bits = (data.getUint32(0) >>> 20) & 0x7FF;
            if (bits === 0) { // denormal
                data.setFloat64(0, value * Math.pow(2, 64));  // exp + 64
                bits = ((data.getUint32(0) >>> 20) & 0x7FF) - 64;
            }
            var exponent = bits - 1022;
            var mantissa = ldexp(value, -exponent);
            return [mantissa, exponent];
        }
        
        const ldexp = function(mantissa, exponent) {
            var steps = Math.min(3, Math.ceil(Math.abs(exponent) / 1023));
            var result = mantissa;
            for (var i = 0; i < steps; i++)
                result *= Math.pow(2, Math.floor((exponent + i) / steps));
            return result;
        }

        wasmImportObject.env["ldexp"] = 
        wasmImportObject.env["MathLdexp"] = function(x,y) {
            return ldexp(x, y)
        };
    }
}