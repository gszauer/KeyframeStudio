#pragma once

enum JsonValueType {
    JSON_NULL = 0,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_STRING,
    JSON_OBJECT,
    JSON_ARRAY
};

struct JsonValue {
    union {
        double asNumber;
        bool asBool;
        struct {
            char* chars;
            unsigned int length;
        } asString;
        struct {
            unsigned int count;
            unsigned int capacity;
            char** names;
            JsonValue** values;
        } asObject;
        struct {
            unsigned int count;
            unsigned int capacity;
            JsonValue** values;
        } asArray;
    };
    JsonValueType type;
};

struct JsonIterator {
    // Iterator state
    JsonValue* object; // what is being iterated?
    unsigned int current;
    bool valid;
    // Iterator access
    union {
        char* name;
        unsigned int index;
    };
    JsonValue* value;
};


JsonValue* JsonParseString(const char* string, unsigned int strLength);
void JsonFree(JsonValue* root);

JsonValue* JsonNewObject();
JsonValue* JsonNewArray();
JsonValue* JsonNewNull();
JsonValue* JsonNewNumber(double value);
JsonValue* JsonNewBool(bool value);
JsonValue* JsonNewString(const char* value);

JsonIterator JsonGetIterator(JsonValue* object);
void JsonIteratorAdvance(JsonIterator* iter);

JsonValue* JsonObjectGet(JsonValue* object, const char* propName);
void JsonObjectSet(JsonValue* object, const char* propName, JsonValue* value);

JsonValue* JsonArrayGet(JsonValue* object, unsigned int index);
void JsonArraySet(JsonValue* object, unsigned int index, JsonValue* value);
void JsonArrayPush(JsonValue* object, JsonValue* value);
