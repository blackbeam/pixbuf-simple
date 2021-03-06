#ifndef NODE_PIXBUF_H_
#define NODE_PIXBUF_H_

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cstring> /* memcpy */

namespace node {
    class Pixbuf : public node::ObjectWrap {
    public:
        ~Pixbuf() {
            v8::V8::AdjustAmountOfExternalAllocatedMemory(-(this->getLength() + sizeof(GdkPixbuf*)));
            g_object_unref(pixbuf);
        };
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments &args);
        static inline bool HasInstance(v8::Handle<v8::Value> val) {
            if (!val->IsObject()) return false;
            v8::Local<v8::Object> obj = val->ToObject();
            return constructor_template->HasInstance(obj);
        }

        static v8::Handle<v8::Value> toImage(const v8::Arguments &args);
        static v8::Handle<v8::Value> scale(const v8::Arguments &args);
        static v8::Handle<v8::Value> drawGlyph(const v8::Arguments &args);

        GdkColorspace getColorspace() { return gdk_pixbuf_get_colorspace(pixbuf); };
        int getNChannels() { return gdk_pixbuf_get_n_channels(pixbuf); };
        gboolean hasAlpha() { return gdk_pixbuf_get_has_alpha(pixbuf); };
        int getBPS() { return gdk_pixbuf_get_bits_per_sample(pixbuf); };
        guchar* getPixels() { return gdk_pixbuf_get_pixels(pixbuf); };
        int getWidth() { return gdk_pixbuf_get_width(pixbuf); };
        int getHeight() { return gdk_pixbuf_get_height(pixbuf); };
        int getRowstride() { return gdk_pixbuf_get_rowstride(pixbuf); };
        const gchar* getOption(const gchar *key) { return gdk_pixbuf_get_option(pixbuf, key); };
        long int getLength() { return gdk_pixbuf_get_height( pixbuf ) * gdk_pixbuf_get_rowstride( pixbuf ); };
        GdkPixbuf *getPixbuf() { return pixbuf; };
        void setPixbuf(GdkPixbuf* newpb) {
            g_object_unref(pixbuf);
            pixbuf = newpb;
        }

    private:
        static void afterRender(uv_work_t* work_req, int status);
        static void render(uv_work_t* work_req);
        static void afterDrawGlyph(uv_work_t* work_req, int status);
        static void _drawGlyph(uv_work_t* work_req);
        static v8::Persistent<v8::FunctionTemplate> constructor_template;

        static v8::Handle<v8::Value> getPixel(unsigned int index, const v8::AccessorInfo &info);
        static v8::Handle<v8::Value> setPixel(unsigned int index, v8::Local<v8::Value> value, const v8::AccessorInfo &info);
        static v8::Handle<v8::Boolean> checkPixel(unsigned int index, const v8::AccessorInfo &info);
        static v8::Handle<v8::Array> enumeratePixel(const v8::AccessorInfo &info);

        static v8::Handle<v8::Value> paramsGetter(v8::Local<v8::String> property, const v8::AccessorInfo &info);

        static void parseRenderOptions(v8::Local<v8::Value> options, gchar*** keys, gchar*** values, uint32_t *optc);

        static inline void freePixels(guchar *pixels, gpointer data) {
            g_free(pixels);
        }

        Pixbuf(GdkPixbuf *source) : ObjectWrap() {
            pixbuf = gdk_pixbuf_copy(source);
            long int pixbufLength = ( gdk_pixbuf_get_height( pixbuf ) * gdk_pixbuf_get_rowstride( pixbuf ) );
            long int dataSize = sizeof(GdkPixbuf*) + pixbufLength;
            v8::V8::AdjustAmountOfExternalAllocatedMemory(dataSize);
        }

        Pixbuf(guchar * pixels, bool has_alpha, int width, int height) : ObjectWrap() {
            int rowstride = has_alpha ? width * 4 : width * 3;
            rowstride = (rowstride + 3) & ~3;
            guchar* pixdata = (guchar*) g_malloc(height * rowstride);
            memcpy(pixdata, pixels, height * rowstride);
            pixbuf = gdk_pixbuf_new_from_data(
                pixdata,
                GDK_COLORSPACE_RGB,
                has_alpha,
                8,
                width, height,
                rowstride,
                freePixels, NULL);
            long int pixbufLength = ( gdk_pixbuf_get_height( pixbuf ) * gdk_pixbuf_get_rowstride( pixbuf ) );
            long int dataSize = sizeof(GdkPixbuf*) + pixbufLength;
            v8::V8::AdjustAmountOfExternalAllocatedMemory(dataSize);
        }

        Pixbuf(gboolean has_alpha, int width, int height) : ObjectWrap() {
            pixbuf = gdk_pixbuf_new(
            /*Currently support only*/ GDK_COLORSPACE_RGB,
                has_alpha,
            /*Currently support only*/ 8,
                width,
                height
            );
            long int pixbufLength = ( gdk_pixbuf_get_height( pixbuf ) * gdk_pixbuf_get_rowstride( pixbuf ) );
            long int dataSize = sizeof(GdkPixbuf*) + pixbufLength;
            v8::V8::AdjustAmountOfExternalAllocatedMemory(dataSize);
        };

        gboolean save(const char *type);

        GdkPixbuf *pixbuf;
    };
}
#endif
