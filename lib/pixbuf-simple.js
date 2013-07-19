try {
    module.exports = require('../build/Debug/pixbuf').Pixbuf;
} catch (e) {
    try {
        module.exports = require('../build/Release/pixbuf').Pixbuf;
    } catch (e2) {
        module.exports = require('../build/default/pixbuf').Pixbuf;
    }
}