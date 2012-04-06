try {
    module.exports = require('../build/Release/pixbuf').Pixbuf;
} catch (e) {
    module.exports = require('../build/default/pixbuf').Pixbuf;
}