#include "Pixbuf.hh"

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>

#include <cstring> /* memcpy */
#include <cstdlib>

using namespace node;
using namespace v8;

Persistent<FunctionTemplate> Pixbuf::constructor_template;
static Persistent<String> red_sym;
static Persistent<String> green_sym;
static Persistent<String> blue_sym;
static Persistent<String> alpha_sym;

namespace node {

    Handle<Value> Pixbuf::New(const Arguments &args) {
        HandleScope scope;
        Pixbuf *pb;

        /* new Pixbuf(pixels: Buffer, has_alpha: bool, width: Integer, height: Integer) */
        if (args.Length() == 4 && Buffer::HasInstance(args[0]) && args[1]->IsBoolean() && args[2]->IsInt32() && args[3]->IsInt32()) {
            guchar *pixels = (guchar*) Buffer::Data( args[0]->ToObject() );
            bool has_alpha = args[1]->ToBoolean()->Value();
            int width = args[2]->ToInt32()->Value();
            int height = args[3]->ToInt32()->Value();
            pb = new Pixbuf(pixels, has_alpha, width, height);
        }
        /* new Pixbuf(has_alpha: bool, width: Integer, height: Integer) */
        else if (args.Length() == 3 && args[0]->IsBoolean() && args[1]->IsInt32() && args[2]->IsInt32()) {
            bool has_alpha = args[0]->ToBoolean()->Value();
            int width = args[1]->ToInt32()->Value();
            int height = args[2]->ToInt32()->Value();
            pb = new Pixbuf(has_alpha, width, height);
        }
        /* new Pixbuf(fileName: String, [width: Number, [height: Number, [preserv_aspect_ratio: bool]]]) */
        else if ( 0 < args.Length() && args.Length() < 5 &&
		args[0]->IsString() &&
		! ( args.Length() > 1 && ! args[1]->IsNumber() ) &&
		! ( args.Length() > 2 && ! args[2]->IsNumber() ) &&
		! ( args.Length() > 3 && ! args[3]->IsBoolean() ) ) {
            GError *err = NULL;
            String::Utf8Value fileName(args[0]);
	    GdkPixbuf *tmp;
	    if ( args.Length() == 1 ) {
	      tmp = gdk_pixbuf_new_from_file(*fileName, &err);
	    } else {
		int width = -1, height = -1;
		gboolean preserv_aspect_ratio = true;
		if ( args.Length() > 3 ) preserv_aspect_ratio = args[3]->ToBoolean()->Value();
		if ( args.Length() > 2 ) height = static_cast<int> ( args[2]->ToNumber()->Value() );
		if ( args.Length() > 1 ) width = static_cast<int> ( args[1]->ToNumber()->Value() );
		tmp = gdk_pixbuf_new_from_file_at_scale( *fileName, width, height, preserv_aspect_ratio, &err );
	    }
	    if (!tmp) {
		Local< String > errStr = String::New(err->message);
		g_clear_error(&err);
		return ThrowException(Exception::Error(errStr));
	    }
	    pb = new Pixbuf( gdk_pixbuf_get_pixels( tmp ),
		    gdk_pixbuf_get_has_alpha( tmp ),
		    gdk_pixbuf_get_width( tmp ),
		    gdk_pixbuf_get_height( tmp ) );
	} else {
            return ThrowException(Exception::TypeError(String::New("Wrong arguments.")));
        }
        pb->Wrap(args.This());

        return args.This();
    }

    void Pixbuf::Initialize(Handle<Object> target) {
        HandleScope scope;

        red_sym = Persistent<String>::New(String::NewSymbol("r"));
        green_sym = Persistent<String>::New(String::NewSymbol("g"));
        blue_sym = Persistent<String>::New(String::NewSymbol("b"));
        alpha_sym = Persistent<String>::New(String::NewSymbol("a"));

        Local<FunctionTemplate> t = FunctionTemplate::New(Pixbuf::New);
        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
        constructor_template->SetClassName(String::NewSymbol("Pixbuf"));

        constructor_template->InstanceTemplate()->SetIndexedPropertyHandler(
                Pixbuf::getPixel, Pixbuf::setPixel, 0, Pixbuf::checkPixel, Pixbuf::enumeratePixel);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("pixels"), Pixbuf::paramsGetter);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("has_alpha"), Pixbuf::paramsGetter);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("width"), Pixbuf::paramsGetter);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("height"), Pixbuf::paramsGetter);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("length"), Pixbuf::paramsGetter);

        NODE_SET_PROTOTYPE_METHOD(constructor_template, "toImage", Pixbuf::toImage);

        target->Set(String::NewSymbol("Pixbuf"), constructor_template->GetFunction());
    }

    Handle<Value> Pixbuf::paramsGetter(Local< String > property, const AccessorInfo &info) {
        HandleScope scope;

        String::Utf8Value propName(property);
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());

        if (strcmp(*propName, "pixels") == 0) {
            Buffer *pixels = Buffer::New(self->getLength());
            memcpy( Buffer::Data( pixels ), self->getPixels(), self->getLength());
            return scope.Close(pixels->handle_);
        }

        if (strcmp(*propName, "has_alpha") == 0) {
            return scope.Close(Boolean::New(self->hasAlpha()));
        }

        if (strcmp(*propName, "width") == 0) {
            return scope.Close(Uint32::New(self->getWidth()));
        }

        if (strcmp(*propName, "height") == 0) {
            return scope.Close(Uint32::New(self->getHeight()));
        }

        if (strcmp(*propName, "length") == 0) {
            return scope.Close(Uint32::New(self->getWidth() * self->getHeight()));
        }

        return Handle< Value > ();
    }

    Handle<Value> Pixbuf::toImage(const Arguments &args) {
        HandleScope scope;
        Pixbuf *src = ObjectWrap::Unwrap<Pixbuf>(args.This());
        gchar *buf;
        gsize bufsize;
        GError *err = NULL;
        gchar **keys = NULL;
        gchar **values = NULL;
        int keyn = 0;

        if (args.Length() < 1)
            return ThrowException(Exception::Error(String::New("Wrong arguments: (type: String[, params: Object]).")));
        if (!args[0]->IsString())
            return ThrowException(Exception::TypeError(String::New("`type' must be an instance of String.")));

        String::Utf8Value typestr(args[0]);

        if (args.Length() == 2) {
            if (!args[1]->IsObject())
                return ThrowException(Exception::TypeError(String::New("`params' must be an instance of Object.")));

            Local<v8::Object> params = args[1]->ToObject();
            Local<Array> v8keys = params->GetPropertyNames();
            uint32_t len = v8keys->Length();
            if (len != 0) {
                for (int i = 0; i < len; i++) {
                    Local<String> v8key = Local<String>::Cast(v8keys->Get(i));
                    String::Utf8Value keystr(v8key);
                    Local<Value> v8value = params->Get(v8key);
                    if (v8value->IsString()) {
                        String::Utf8Value valuestr(v8value);
                        keyn++;
                        keys = (gchar **)realloc(keys, sizeof(gchar *) * keyn+1);
                        values = (gchar **)realloc(values, sizeof(gchar *) * keyn+1);
                        keys[keyn-1] = g_strdup(*keystr);
                        values[keyn-1] = g_strdup(*valuestr);
                        keys[keyn] = values[keyn] = NULL;
                    }
                }

            }
        }
        gboolean isOk = true;
        if (keys != NULL) {
            isOk = gdk_pixbuf_save_to_bufferv(src->getPixbuf(), &buf, &bufsize, *typestr, keys, values, &err);
            for ( int i = 0; i < keyn+1; i++ ) {
                free( keys[i] );
                free( values[i] );
            }
            if ( keys != NULL ) free( keys );
            if ( values != NULL ) free( values );
        } else {
            isOk = gdk_pixbuf_save_to_buffer(src->getPixbuf(), &buf, &bufsize, *typestr, &err, NULL);
        }

        if (!isOk) {
            return ThrowException(Exception::Error(String::New(err->message)));
        }

        Buffer *buffer = Buffer::New(bufsize);
        memcpy( Buffer::Data( buffer ), buf, bufsize);

        g_free(buf);

        return scope.Close(buffer->handle_);
    }

    Handle<Array> Pixbuf::enumeratePixel(const AccessorInfo &info) {
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());
        HandleScope scope;
        int len = self->getWidth() * self->getHeight();

        Local<Array> a = Array::New(len);
        for (int i = 0; i < len; i++) {
            a->Set(i, Uint32::New(i));
        }
        return scope.Close(a);
    }

    Handle<Boolean> Pixbuf::checkPixel(uint32_t index, const AccessorInfo &info) {
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());
        HandleScope scope;

        if (index > self->getWidth() * self->getHeight() - 1)
            return Boolean::New(false);
        return Boolean::New(true);
    }

    Handle<Value> Pixbuf::getPixel(uint32_t index, const AccessorInfo &info) {
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());
        HandleScope scope;

        if (index > self->getWidth() * self->getHeight() - 1)
            return Handle<Value>();

        Local<v8::Object> p = v8::Object::New();

        int y = index / self->getHeight();
        int x = index % self->getWidth();

        guchar* pix = self->getPixels() + y * self->getRowstride() + x * self->getNChannels();

        p->Set(red_sym, Uint32::New(pix[0]));
        p->Set(green_sym, Uint32::New(pix[1]));
        p->Set(blue_sym, Uint32::New(pix[2]));;
        if (self->hasAlpha()) {
            p->Set(alpha_sym, Uint32::New(pix[3]));
        }
        return scope.Close(p);
    }

    Handle<Value> Pixbuf::setPixel(uint32_t index, Local<Value> value, const AccessorInfo &info) {
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());
        HandleScope scope;
        guchar* pix;
        Local<Object> p;

        if (index > self->getWidth() * self->getHeight() - 1)
            return Handle<Value>();
        if (!value->IsObject()) {
            goto error;
        }
        p = value->ToObject();
        goto ok;

error:

        if (self->hasAlpha())
            return ThrowException(Exception::TypeError(String::New("Pixel must be an object: {r: Integer, g: Integer, b: Integer, a: Integer}")));
        else
            return ThrowException(Exception::TypeError(String::New("Pixel must be an object: {r: Integer, g: Integer, b: Integer}")));

ok:

        int y = index / self->getHeight();
        int x = index % self->getWidth();

        pix = self->getPixels() + y * self->getRowstride() + x * self->getNChannels();
        if (self->hasAlpha()) {
            if (p->Has(alpha_sym) && p->Get(alpha_sym)->IsUint32()) pix[3] = p->Get(alpha_sym)->ToUint32()->Value();
        }
        if (p->Has(red_sym) && p->Get(red_sym)->IsUint32()) pix[0] = p->Get(red_sym)->ToUint32()->Value();
        if (p->Has(green_sym) && p->Get(green_sym)->IsUint32()) pix[1] = p->Get(green_sym)->ToUint32()->Value();
        if (p->Has(blue_sym) && p->Get(blue_sym)->IsUint32()) pix[2] = p->Get(blue_sym)->ToUint32()->Value();

        return info.This()->Get(index);
    }

}

// Exporting function
    extern "C" void
init (v8::Handle<v8::Object> target)
{
    HandleScope scope;

    //Required for gdk
    g_type_init();

    Pixbuf::Initialize(target);
}
