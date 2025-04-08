function hasProperty(obj, key) {
    return Object.prototype.hasOwnProperty.call(obj, key);
}

function getRawValue(rawValue, name) {
    if (name.length == 0) {
        return "";
    }
    if (rawValue === null) {
        return "";
    }
    for (var i = 0; i < name.length; i++) {
        if (hasProperty(rawValue, name[i])) {
            rawValue = rawValue[name[i]];
        } else {
            return "";
        }
    }
    return rawValue;
}

function setRawValue(rawValue, name, value) {
    for (var i = 0; i < name.length; i++) {
        if (i + 1 == name.length) {
            rawValue[name[i]] = value;
        } else {
            if (!hasProperty(rawValue, name[i])) {
                rawValue[name[i]] = {};
            }
            rawValue = rawValue[name[i]];
        }
    }
}
