#include "gabor_json.h"
#include <stdlib.h>     /* strtod */
#include <string.h>

#define GABOR_JSON_ASSERT(cond)                 \
    if (!(cond)) {                              \
        unsigned char* ptr = (unsigned char*)0; \
        *ptr = 0;                               \
    }                                                               

#define GABOR_JSON_ITER_NEXT()              \
    state->current += 1;                    \
    if (state->current < state->length) {   \
    iter = state->target[state->current];} 

#define GABOR_JSON_ITER_NEXT_LOOP()         \
    state->current += 1;                    \
    if (state->current == state->length) {  \
        break; }                            \
    iter = state->target[state->current];  

struct GaborJsonParseState {
    const char* target;
    unsigned int current;
    unsigned int length;
};

JsonValue* GaborJsonParseValue(GaborJsonParseState* state);
JsonValue* GaborJsonParseString(GaborJsonParseState* state);
JsonValue* GaborJsonParseLiteral(GaborJsonParseState* state);
JsonValue* GaborJsonParseNumber(GaborJsonParseState* state);
    JsonValue* GaborJsonParseWhitespace(GaborJsonParseState* state) {
    char iter = state->target[state->current];
    while (iter == '\t' || iter == '\n' || iter == ' ' || iter == '\r') {
        state->current += 1;
        if (state->current == state->length) {
            break;
        }
        iter = state->target[state->current];
    }

    return 0;
}

char* GaborJsonParserAsciiString(GaborJsonParseState* state) {
    char iter = state->target[state->current];
    if (iter == '"') {
        GABOR_JSON_ITER_NEXT()
        unsigned int start = state->current;
        while (iter != '"') {
            // Escape Character
            if (iter == '\\') {
                GABOR_JSON_ITER_NEXT_LOOP();

                if (iter == '"' || iter == '\\' || iter == '/' ||
                    iter == 'b' || iter == 'f' || iter == 'n' ||
                    iter == 'r' || iter == 't') {
                    GABOR_JSON_ITER_NEXT_LOOP();
                }
                else if (iter == 'u') {
                    GABOR_JSON_ITER_NEXT_LOOP();
                    // 4 hex digits
                    GABOR_JSON_ITER_NEXT_LOOP();
                    GABOR_JSON_ITER_NEXT_LOOP();
                    GABOR_JSON_ITER_NEXT_LOOP();
                    GABOR_JSON_ITER_NEXT_LOOP();
                }
                else {
                    GABOR_JSON_ASSERT("Invalid escape character" == 0);
                }
            }
            else {
                GABOR_JSON_ITER_NEXT_LOOP();
            }
        }
        if (iter != '"') {
            GABOR_JSON_ASSERT("Expected string to end with double qoute" == 0);
        }
        unsigned int end = state->current;
        GABOR_JSON_ITER_NEXT()

        GABOR_JSON_ASSERT(end != start);

        int bytes = (end - start) + 1;
        char* result = (char*)malloc(bytes);
        for (int i = 0; i < (int)(end - start); ++i) {
            result[i] = state->target[start + i];
        }
        result[end - start] = '\0';

        return result;
    }

    GABOR_JSON_ASSERT("Invalid string literal" == 0);
    return 0;
}

JsonValue* GaborJsonParseObject(GaborJsonParseState* state) {
    char iter = state->target[state->current];
    if (iter != '{') {
        GABOR_JSON_ASSERT("Object must start with brace" == 0);
        return 0;
    }
    GABOR_JSON_ITER_NEXT()
    GaborJsonParseWhitespace(state);

    JsonValue* result = JsonNewObject();
    iter = state->target[state->current];
    while (iter != '}') {
        GaborJsonParseWhitespace(state);
        char* key = GaborJsonParserAsciiString(state);
        if (key == 0) {
            break;
        }
        GaborJsonParseWhitespace(state);
        iter = state->target[state->current];

        if (iter != ':') {
            GABOR_JSON_ASSERT("Object expects : after key" == 0);
            break;
        }
        else {
            GABOR_JSON_ITER_NEXT_LOOP();
        }

        JsonValue* value = GaborJsonParseValue(state);
        if (value == 0) {
            break;
        }

        JsonObjectSet(result, key, value);


        iter = state->target[state->current];
        if (iter == ',') {
            GABOR_JSON_ITER_NEXT_LOOP();
        }
        else {
            break;
        }
        iter = state->target[state->current];
    }

    iter = state->target[state->current];
    if (iter != '}') {
        GABOR_JSON_ASSERT("Object must end with brace" == 0);
        JsonFree(result);
        return 0;
    }
    GABOR_JSON_ITER_NEXT()

    return result;
}

JsonValue* GaborJsonParseArray(GaborJsonParseState* state) {
    char iter = state->target[state->current];
    if (iter != '[') {
        GABOR_JSON_ASSERT("Array must start with bracket" == 0);
        return 0;
    }
    GABOR_JSON_ITER_NEXT()
    GaborJsonParseWhitespace(state);

    JsonValue* result = JsonNewArray();
    iter = state->target[state->current];
    while (iter != ']') {
        GaborJsonParseWhitespace(state);
        JsonArrayPush(result, GaborJsonParseValue(state));
        iter = state->target[state->current];
        if (iter == ',') {
            GABOR_JSON_ITER_NEXT_LOOP();
        }
        else {
            break;
        }
        iter = state->target[state->current];
    }

    iter = state->target[state->current];
    if (iter != ']') {
        GABOR_JSON_ASSERT("Array must end with bracket" == 0);
        JsonFree(result);
        return 0;
    }
    GABOR_JSON_ITER_NEXT()

    return result;
}

JsonValue* GaborJsonParseValue(GaborJsonParseState* state) {
    JsonValue* result = 0;

    GaborJsonParseWhitespace(state);
    char iter = state->target[state->current];
    if (iter == '"') {
        result = GaborJsonParseString(state);
    }
    else if (iter == '{') {
        result = GaborJsonParseObject(state);
    }
    else if (iter == '[') {
        result = GaborJsonParseArray(state);
    }
    else if (iter == 't' || iter == 'f' || iter == 'n') {
        result = GaborJsonParseLiteral(state);
    }
    else if (iter == '-' || (iter >= '0' && iter <= '9')) {
        result = GaborJsonParseNumber(state);
    }
    else {
        GABOR_JSON_ASSERT("Expected json value, invalid starting character" == 0);
    }

    GaborJsonParseWhitespace(state);

    return result;
}

JsonValue* GaborJsonParseString(GaborJsonParseState* state) {
    char* result = GaborJsonParserAsciiString(state);
    if (result == 0) {
        return 0;
    }

    return JsonNewString(result);
}

JsonValue* GaborJsonParseNumber(GaborJsonParseState* state) {
    char iter = state->target[state->current];
    int start = state->current;

    bool isNegative = false;
    bool hasDot = false;

    if (iter == '-') {
        GABOR_JSON_ASSERT(!isNegative);
        if (isNegative) { return 0; };
        isNegative = true;
        GABOR_JSON_ITER_NEXT()
    }

    while (true) {
        if (iter >= '0' && iter <= '9') {
            GABOR_JSON_ITER_NEXT_LOOP();
        }
        else if (iter == '.') {
            GABOR_JSON_ASSERT(!hasDot);
            if (hasDot) { return 0; };
            hasDot = true;
            GABOR_JSON_ITER_NEXT_LOOP();
        }
        else {
            break;
        }
    }

    /*int end = state->current;
    char* buffer = malloc(end - start + 1);
    memcpy(buffer, &state->target[start], end - start);
    buffer[end - start] = 0;*/

    double value = strtod(&state->target[start], 0);
    return JsonNewNumber(value);
}

JsonValue* GaborJsonParseLiteral(GaborJsonParseState* state) {
    char iter = state->target[state->current];

    if (iter == 't') {
        GABOR_JSON_ITER_NEXT()
        if (iter == 'r') {
            GABOR_JSON_ITER_NEXT()
            if (iter == 'u') {
                GABOR_JSON_ITER_NEXT()
                if (iter == 'e') {
                    GABOR_JSON_ITER_NEXT()
                    return JsonNewBool(true);
                }
            }
        }
    }
    else if (iter == 'f') {
        GABOR_JSON_ITER_NEXT()
        if (iter == 'a') {
            GABOR_JSON_ITER_NEXT()
            if (iter == 'l') {
                GABOR_JSON_ITER_NEXT()
                if (iter == 's') {
                    GABOR_JSON_ITER_NEXT()
                    if (iter == 'e') {
                        GABOR_JSON_ITER_NEXT()
                        return JsonNewBool(false);
                    }
                }
            }
        }
    }
    else if (iter == 'n') {
        GABOR_JSON_ITER_NEXT();
        if (iter == 'u') {
            GABOR_JSON_ITER_NEXT();
            if (iter == 'l') {
                GABOR_JSON_ITER_NEXT();
                if (iter == 'l') {
                    GABOR_JSON_ITER_NEXT();
                    return JsonNewNull();
                }
            }
        }
    }

    GABOR_JSON_ASSERT("literal must be true, false, or null" == 0);
    return 0;
}

JsonValue* JsonNewObject() {
    JsonValue* result = (JsonValue * )malloc(sizeof(JsonValue));
    memset(result, 0, sizeof(JsonValue));
    result->type = JsonValueType::JSON_OBJECT;

    const unsigned int INITIAL_CAPACITY = 5;
    result->asObject.count = 0;
    result->asObject.capacity = INITIAL_CAPACITY;
    result->asObject.names = (char**)malloc(sizeof(char*) * INITIAL_CAPACITY);
    memset(result->asObject.names, 0, sizeof(char*) * INITIAL_CAPACITY);
    result->asObject.values = (JsonValue**)malloc(sizeof(JsonValue*) * INITIAL_CAPACITY);
    memset(result->asObject.values, 0, sizeof(JsonValue*) * INITIAL_CAPACITY);
    return result;
}

JsonValue* JsonNewArray() {
    JsonValue* result = (JsonValue*)malloc(sizeof(JsonValue));
    memset(result, 0, sizeof(JsonValue));
    result->type = JsonValueType::JSON_ARRAY;

    const unsigned int INITIAL_CAPACITY = 5;
    result->asArray.count = 0;
    result->asArray.capacity = INITIAL_CAPACITY;
    result->asArray.values = (JsonValue**)malloc(sizeof(JsonValue*) * INITIAL_CAPACITY);
    memset(result->asArray.values, 0, sizeof(JsonValue*) * INITIAL_CAPACITY);
    return result;
}

JsonValue* JsonNewNull() {
    JsonValue* result = (JsonValue*)malloc(sizeof(JsonValue));
    memset(result, 0, sizeof(JsonValue));
    result->type = JsonValueType::JSON_NULL;
    return result;
}

JsonValue* JsonNewNumber(double value) {
    JsonValue* result = (JsonValue*)malloc(sizeof(JsonValue));
    memset(result, 0, sizeof(JsonValue));
    result->type = JsonValueType::JSON_NUMBER;
    result->asNumber = value;
    return result;
}

JsonValue* JsonNewBool(bool value) {
    JsonValue* result = (JsonValue*)malloc(sizeof(JsonValue));
    memset(result, 0, sizeof(JsonValue));
    result->type = JsonValueType::JSON_BOOLEAN;
    result->asBool = value;
    return result;
}

JsonValue* JsonNewString(const char* value) {
    JsonValue* result = (JsonValue*)malloc(sizeof(JsonValue));
    memset(result, 0, sizeof(JsonValue));
    result->type = JsonValueType::JSON_STRING;

    size_t length = strlen(value);
    result->asString.chars = (char*)malloc(sizeof(char) * (length + 1));
    memcpy(result->asString.chars, value, length);
    result->asString.chars[length] = 0;

    return result;
}

void GaborJsonFreeObject(JsonValue* value) {
    for (unsigned int i = 0, size = value->asObject.count; i < size; ++i) {
        JsonFree(value->asObject.values[i]);
        if (value->asObject.names[i] != 0) {
            free(value->asObject.names[i]);
        }
    }

    if (value->asObject.values != 0) {
        free(value->asObject.values);
    }
    value->asObject.values = 0;

    if (value->asObject.names != 0) {
        free(value->asObject.names);
    }
    value->asObject.names = 0;

    value->asObject.count = 0;
    value->asObject.capacity = 0;
}

void GaborJsonFreeArray(JsonValue* value) {
    for (unsigned int i = 0, size = value->asArray.count; i < size; ++i) {
        JsonFree(value->asArray.values[i]);
    }
    if (value->asArray.values != 0) {
        free(value->asArray.values);
    }
    value->asArray.values = 0;
    value->asArray.count = 0;
    value->asArray.capacity = 0;
}

void GaborJsonFreeString(JsonValue* value) {
    if (value->asString.chars != 0) {
        free(value->asString.chars);
    }
    value->asString.chars = 0;
    value->asString.length = 0;
}

void JsonFree(JsonValue* root) {
    if (root == 0) {
        return;
    }

    if (root->type == JsonValueType::JSON_OBJECT) {
        GaborJsonFreeObject(root);
    }
    else if (root->type == JsonValueType::JSON_ARRAY) {
        GaborJsonFreeArray(root);
    }
    else if (root->type == JsonValueType::JSON_STRING) {
        GaborJsonFreeString(root);
    }

    free(root);
}

JsonValue* JsonObjectGet(JsonValue* object, const char* propName) {
    GABOR_JSON_ASSERT(object != 0 && object->type == JsonValueType::JSON_OBJECT);
    GABOR_JSON_ASSERT(propName != 0);

    for (unsigned int i = 0, size = object->asObject.count; i < size; ++i) {
        if (object->asObject.names[i] != 0) {
            if (strcmp(object->asObject.names[i], propName) == 0) {
                return object->asObject.values[i];
            }
        }
    }

    return 0;
}

void JsonArraySet(JsonValue* object, unsigned int index, JsonValue* value) {
    GABOR_JSON_ASSERT(object != 0 && object->type == JsonValueType::JSON_ARRAY);
    GABOR_JSON_ASSERT(index < object->asArray.count && value != 0);

    if (object->asArray.values[index] != 0) {
        JsonFree(object->asArray.values[index]);
    }
    object->asArray.values[index] = value;
}

void JsonArrayPush(JsonValue* object, JsonValue* value) {
    GABOR_JSON_ASSERT(object != 0 && object->type == JsonValueType::JSON_ARRAY);
    GABOR_JSON_ASSERT(value != 0);

    // Increase capacity if we must
    if (object->asArray.count + 1 >= object->asArray.capacity) {
        unsigned int newCap = object->asArray.capacity * 2;
        JsonValue** newValues = (JsonValue**)malloc(sizeof(JsonValue*) * newCap);
        memset(newValues, 0, sizeof(JsonValue*) * newCap);
        memcpy(newValues, object->asArray.values, sizeof(JsonValue*) * object->asArray.count);

        if (object->asArray.values != 0) {
            free(object->asArray.values);
        }
        object->asArray.values = newValues;

        object->asArray.capacity = newCap;
    }

    object->asArray.values[object->asArray.count++] = value;
}

void JsonObjectSet(JsonValue* object, const char* propName, JsonValue* value) {
    GABOR_JSON_ASSERT(object != 0 && object->type == JsonValueType::JSON_OBJECT);
    GABOR_JSON_ASSERT(propName != 0 && value != 0);

    for (unsigned int i = 0, size = object->asObject.count; i < size; ++i) {
        if (object->asObject.names[i] != 0) {
            if (strcmp(object->asObject.names[i], propName) == 0) {
                if (object->asObject.values[i] != 0) {
                    JsonFree(object->asObject.values[i]);
                }
                object->asObject.values[i] = value;
                return;
            }
        }
    }
    
    // Increase capacity if we must
    if (object->asObject.count + 1 >= object->asObject.capacity) {
        unsigned int newCap = object->asObject.capacity * 2;
        char** newNames = (char**)malloc(sizeof(char*) * newCap);
        memset(newNames, 0, sizeof(char*) * newCap);
        memcpy(newNames, object->asObject.names, sizeof(char*) * object->asObject.count);
        JsonValue** newValues = (JsonValue**)malloc(sizeof(JsonValue*) * newCap);
        memset(newValues, 0, sizeof(JsonValue*) * newCap);
        memcpy(newValues, object->asObject.values, sizeof(JsonValue*) * object->asObject.count);
        
        if (object->asObject.names != 0) {
            free(object->asObject.names);
        }
        object->asObject.names = newNames;

        if (object->asObject.values != 0) {
            free(object->asObject.values);
        }
        object->asObject.values = newValues;

        object->asObject.capacity = newCap;
    }

    unsigned int count = object->asObject.count;
    unsigned int len = (unsigned int)strlen(propName);
    object->asObject.names[count] = (char*)malloc(sizeof(char) * (len + 1));
    memcpy(object->asObject.names[count], propName, sizeof(char) * len);
    object->asObject.names[count][len] = '\0';
    object->asObject.values[count] = value;

    object->asObject.count += 1;
}

JsonValue* JsonArrayGet(JsonValue* object, unsigned int index) {
    GABOR_JSON_ASSERT(object != 0 && object->type == JsonValueType::JSON_ARRAY);
    GABOR_JSON_ASSERT(index < object->asArray.count);

    if (index >= object->asArray.count) {
        return 0;
    }

    return object->asArray.values[index];
}

JsonIterator JsonGetIterator(JsonValue* object) {
    JsonIterator result = { 0 };
    // Current, value, name, and index are all 0

    result.valid = object != 0 && (object->type == JsonValueType::JSON_OBJECT || object->type == JsonValueType::JSON_ARRAY);
    result.object = object;

    if (result.valid && object->type == JsonValueType::JSON_ARRAY) {
        if (object->asArray.count == 0) {
            result.valid = false;
        }
    }

    if (result.valid && object->type == JsonValueType::JSON_OBJECT) {
        if (object->asObject.count == 0) {
            result.valid = false;
        }
    }

    if (result.valid) {
        if (object->type == JsonValueType::JSON_OBJECT) {
            unsigned int count = object->asObject.count;
            if (object->asObject.names != 0 && count > 0) {
                result.name = object->asObject.names[0];
            }
            if (object->asObject.values != 0 && count > 0) {
                result.value = object->asObject.values[0];
            }
        }
        else if (object->type == JsonValueType::JSON_ARRAY) {
            unsigned int count = object->asArray.count;
            result.index = 0;
            if (object->asArray.values != 0 && count > 0) {
                result.value = object->asArray.values[0];
            }
        }
    }

    return result;
}
void JsonIteratorAdvance(JsonIterator* iter) {
    GABOR_JSON_ASSERT(iter != 0 && iter->valid && iter->value != 0);
    GABOR_JSON_ASSERT(iter->object != 0 && (iter->object->type == JsonValueType::JSON_OBJECT || iter->object->type == JsonValueType::JSON_ARRAY));

    iter->current += 1;

    if (iter->object->type == JsonValueType::JSON_OBJECT) {
        iter->valid = iter->current < iter->object->asObject.count;
        if (iter->valid) {
            iter->name = iter->object->asObject.names[iter->current];
            iter->value = iter->object->asObject.values[iter->current];
        }
    }
    else if (iter->object->type == JsonValueType::JSON_ARRAY) {
        iter->valid = iter->current < iter->object->asArray.count;
        if (iter->valid) {
            iter->index = iter->current;
            iter->value = iter->object->asArray.values[iter->index];
        }
    }
    else {
        iter->valid = false;
        GABOR_JSON_ASSERT("Only objects and arrays allowed" == 0);
    }
}

JsonValue* JsonParseString(const char* string, unsigned int strLength) {
    GABOR_JSON_ASSERT(string != 0);

    GaborJsonParseState parseState = { 0 };
    parseState.target = string;
    parseState.current = 0;
    parseState.length = strLength;
    if (strLength == 0) {
        parseState.length = (unsigned int)strlen(string);
    }

    GaborJsonParseWhitespace(&parseState);
    return GaborJsonParseValue(&parseState);
}