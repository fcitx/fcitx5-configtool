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

function flattenProperties(properties, name) {
    var optionProperties = {};
    for (var property in properties) {
        if (property !== name) {
            optionProperties[property] = properties[property];
        }
    }
    if (hasProperty(properties, name)) {
        for (var constrainedProperty in properties[name]) {
            optionProperties[constrainedProperty] =
                properties[name][constrainedProperty];
        }
    }
    return optionProperties;
}
