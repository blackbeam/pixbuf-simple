#ifndef NODE_PIXBUF_H_
#define NODE_PIXBUF_H_

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

namespace node {
    class Pixbuf : public node::ObjectWrap {
	public:
	    ~Pixbuf() {
		v8::V8::AdjustAmountOfExternalAllocatedMemory(-dataSize);
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

	    GdkColorspace getColorspace() { return gdk_pixbuf_get_colorspace(pixbuf); };
	    int getNChannels() { return gdk_pixbuf_get_n_channels(pixbuf); };
	    gboolean hasAlpha() { return gdk_pixbuf_get_has_alpha(pixbuf); };
	    int getBPS() { return gdk_pixbuf_get_bits_per_sample(pixbuf); };
	    guchar* getPixels() { return gdk_pixbuf_get_pixels(pixbuf); };
	    int getWidth() { return gdk_pixbuf_get_width(pixbuf); };
	    int getHeight() { return gdk_pixbuf_get_height(pixbuf); };
	    int getRowstride() { return gdk_pixbuf_get_rowstride(pixbuf); };
	    const gchar* getOption(const gchar *key) { return gdk_pixbuf_get_option(pixbuf, key); };
	    long int getLength() { return pixbufLength; };
	    GdkPixbuf *getPixbuf() { return pixbuf; };

	private:
	    static v8::Persistent<v8::FunctionTemplate> constructor_template;

	    static v8::Handle<v8::Value> getPixel(uint32_t index, const v8::AccessorInfo &info);
	    static v8::Handle<v8::Value> setPixel(uint32_t index, v8::Local<v8::Value> value, const v8::AccessorInfo &info);
	    static v8::Handle<v8::Boolean> checkPixel(uint32_t index, const v8::AccessorInfo &info);
	    static v8::Handle<v8::Array> enumeratePixel(const v8::AccessorInfo &info);

	    static v8::Handle<v8::Value> paramsGetter(v8::Local<v8::String> property, const v8::AccessorInfo &info);

	    Pixbuf(GdkPixbuf *source) : ObjectWrap() {
		pixbuf = gdk_pixbuf_copy(source);
		pixbufLength = ( gdk_pixbuf_get_height( pixbuf ) * gdk_pixbuf_get_rowstride( pixbuf ) );
		dataSize = sizeof(GdkPixbuf*) + pixbufLength;
		v8::V8::AdjustAmountOfExternalAllocatedMemory(dataSize);
	    }

	    Pixbuf(guchar * pixels, bool has_alpha, int width, int height) : ObjectWrap() {
		int rowstride = has_alpha ? width * 4 : width * 3;
		rowstride = (rowstride + 3) & ~3;
		pixbuf = gdk_pixbuf_new_from_data(
			pixels,
			GDK_COLORSPACE_RGB,
			has_alpha,
			8,
			width, height,
			rowstride,
			NULL, NULL);
		pixbufLength = ( gdk_pixbuf_get_height( pixbuf ) * gdk_pixbuf_get_rowstride( pixbuf ) );
		dataSize = sizeof(GdkPixbuf*) + pixbufLength;
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
		pixbufLength = ( gdk_pixbuf_get_height( pixbuf ) * gdk_pixbuf_get_rowstride( pixbuf ) );
		dataSize = sizeof(GdkPixbuf*) + pixbufLength;
		v8::V8::AdjustAmountOfExternalAllocatedMemory(dataSize);
	    };

	    gboolean save(const char *type);

	    GdkPixbuf *pixbuf;
	    long int dataSize;
	    long int pixbufLength;
    };
}
#endif
