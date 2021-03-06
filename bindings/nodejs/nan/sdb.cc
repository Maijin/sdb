/* sdb-nan.js - pancake@nopcode.org // 2015 */

#include <node.h>
#include <nan.h>
#include <sdb.h>

using namespace v8;

class Database : public node::ObjectWrap {
	public:
		static void Init(Handle<Object> exports);
		static NAN_METHOD(Open);
		static NAN_METHOD(Close);
		static NAN_METHOD(Ns);
		static NAN_METHOD(New);
		static NAN_METHOD(Add);
		static NAN_METHOD(Set);
		static NAN_METHOD(UnSet);
		static NAN_METHOD(UnSetLike);
		static NAN_METHOD(Drain);
		static NAN_METHOD(Type);
		static NAN_METHOD(Expire);
		static NAN_METHOD(Reset);
		static NAN_METHOD(Sync);
		static NAN_METHOD(KeysOnDisk);
		static NAN_METHOD(KeysOnMemory);
		static NAN_METHOD(Query);
		static NAN_METHOD(Like);
		static NAN_METHOD(UnLink);
		static NAN_METHOD(Get);
		static NAN_METHOD(GetVersion);
		static NAN_METHOD(Exists);
		Database() {
			this->obj = sdb_new0 ();
		}
		Database(const char *file) {
			this->obj = sdb_new (NULL, file, 0);
		}
		Database(Sdb *db) {
			this->obj = db;
		}
		~Database() {
			sdb_free (this->obj);
		}

	private:
		Sdb *obj;
};

NAN_METHOD(Database::Like) {
	Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
	char **res = NULL;

	NanScope();

	switch (args.Length()) {
	case 1:
		if (args[0]->IsString()) {
			NanUtf8String k (args[0]);
			res = sdb_like(sdb, *k, NULL, NULL);
		} else {
			NanThrowTypeError ("string expected");
		}
		break;
	case 2:
		char *key = NULL, *val = NULL;
		// TODO: fail if type is bool, object, ...
		if (args[0]->IsString()) {
			NanUtf8String k (args[0]);
			key = *k;
		}
		if (args[1]->IsString()) {
			NanUtf8String v (args[1]);
			val = *v;
		}
		res = sdb_like (sdb, key, val, NULL);
		break;
	}
	if (res) {
		Local<Object> obj = NanNew<Object>();
		for (int i = 0; res[i]; i+=2) {
			obj->Set(NanNew(res[i]), NanNew(res[i+1]));
		}
		NanReturnValue(obj);
	}

}

NAN_METHOD(Database::UnSetLike) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("string expected");
		}
		NanUtf8String k (args[0]);
		Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		sdb_unset_like (sdb, *k);
	}
}

NAN_METHOD(Database::Open) {
	int fd = -1;
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("string expected");
		}
		NanUtf8String k (args[0]);
		Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		fd = sdb_open (sdb, *k);
	}
	NanReturnValue(NanNew (fd != -1));
}

NAN_METHOD(Database::Close) {
	/* arguments ignored */
	Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
	sdb_close (sdb);
}

NAN_METHOD(Database::Type) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("string expected");
		}
		NanUtf8String k (args[0]);
		Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		const char *t = sdb_type (sdb_const_get (sdb, *k, 0));
		NanReturnValue(NanNew (t));
	}
}

NAN_METHOD(Database::UnSet) {
	int len = args.Length();
	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("string expected");
		}
		NanUtf8String k (args[0]);
		Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		sdb_unset (sdb, *k, 0);
	}
}

NAN_METHOD(Database::Exists) {
	bool ret = false;
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (args[0]->IsString()) {
			NanUtf8String k (args[0]);
			Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
			int ret = sdb_exists (sdb, *k);
			NanReturnValue(NanNew ((bool)ret));
			// sdb_num_exists
		} else {
			NanThrowTypeError ("string key expected");
		}
	}
	NanReturnValue(NanNew(ret));
}

NAN_METHOD(Database::Set) {
	int len = args.Length();

	NanScope();

	if (len == 2) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		NanUtf8String k (args[0]);
		if (args[1]->IsBoolean()) {
			ut64 v = args[1]->Uint32Value();
			(void)sdb_bool_set (sdb, *k, v, 0);
		} else if (args[1]->IsNumber()) {
			ut64 v = args[1]->Uint32Value();
			(void)sdb_num_set (sdb, *k, v, 0);
		} else if (args[1]->IsString()) {
			NanUtf8String v (args[1]);
			(void)sdb_set (sdb, *k, *v, 0);
		} else {
			NanThrowTypeError ("Second argument must be a string");
		}
	} else {
		NanThrowTypeError ("Sdb.Set Invalid arguments");
	}
	NanReturnThis();
}

NAN_METHOD(Database::Ns) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		if (!args[0]->IsString()) {
			NanThrowTypeError ("namespace must be a string");
		}
		NanUtf8String k (args[0]);
		Sdb *db = sdb_ns (sdb, *k, 0);
		Database *so = new Database(db);
		NanReturnValue(NanNew (so));
	} else if (len == 2) {
		Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		if (!args[0]->IsString()) {
			NanThrowTypeError ("namespace must be a string");
		}
		NanUtf8String k (args[0]);
		ut32 v = args[1]->Uint32Value();
		Sdb *db = sdb_ns (sdb, *k, v);
		Database *so = new Database(db);
		NanReturnValue(NanNew (so));
	} else {
		/* return current namespace */
	}
}

#if 0
NAN_METHOD(Database::Open) {
	/* TODO: call constructor */
}
#endif

NAN_METHOD(Database::Add) {
	int ret = 0;
	int len = args.Length();

	NanScope();

	if (len == 2) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		} else if (args[1]->IsNumber()) {
			NanUtf8String k (args[0]);
			ut64 v = args[1]->Uint32Value();
			Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
			ret = sdb_num_add (sdb, *k, v, 0);
		} else if (args[1]->IsString()) {
			NanUtf8String k (args[0]);
			NanUtf8String v (args[1]);
			Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
			ret = sdb_add (sdb, *k, *v, 0);
		} else {
			NanThrowTypeError ("Second argument must be a string");
		}
	} else {
		NanThrowTypeError ("Sdb.Set Invalid arguments");
	}
	if (ret == 0) {
		NanReturnValue(NanNew((bool)ret));
	} else {
		NanReturnThis();
	}
}

NAN_METHOD(Database::GetVersion) {
	NanScope();
	NanReturnValue(NanNew(SDB_VERSION));
}

NAN_METHOD(Database::Sync) {
	Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
	bool v = sdb_sync (sdb);
	NanScope();
	NanReturnValue(NanNew(v));
}

NAN_METHOD(Database::Drain) {
	int len = args.Length();
	NanScope();
	if (len == 1) {
		//Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		//Sdb *sdb2 = ((Database*)(args[0]->ToObject()->Get(0)))->obj;
		//Sdb *sdb2 = ObjectWrap::Unwrap<Database>(arg)->obj;
		//sdb_drain (sdb, sdb2);
		NanReturnThis();
	} else {
		NanThrowTypeError ("Missing destination database");
	}
}

NAN_METHOD(Database::Reset) {
	Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
	sdb_reset (sdb);
	NanScope();
	NanReturnThis();
}

NAN_METHOD(Database::UnLink) {
	Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
	bool v = sdb_unlink(sdb);
	NanScope();
	NanReturnValue(NanNew(v));
}

NAN_METHOD(Database::Get) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		const char *v = sdb_const_get (sdb, *k, NULL);
		if (v != NULL) {
			NanReturnValue(NanNew(v));
		}
	}
	NanReturnUndefined();
}

NAN_METHOD(Encode) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		char *str = sdb_encode ((const ut8*)*k, -1);
		if (str) {
			Local<String> v = NanNew(str);
			free (str);
			NanReturnValue(v);
		}
	}
}

NAN_METHOD(JsonIndent) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		char *res = sdb_json_indent (*k);
		Local<String> v = NanNew(res);
		free (res);
		NanReturnValue(v);
	}
}

NAN_METHOD(JsonUnindent) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		char *res = sdb_json_unindent (*k);
		Local<String> v = NanNew(res);
		free (res);
		NanReturnValue(v);
	}
}

NAN_METHOD(TypeOf) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		NanReturnValue(NanNew(sdb_type (*k)));
	}
}

NAN_METHOD(Decode) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		char *str = (char *)sdb_decode (*k, NULL);
		if (str) {
			Local<String> v = NanNew(str);
			free (str);
			NanReturnValue(v);
		}
	}
}

NAN_METHOD(Database::Query) {
	int len = args.Length();

	NanScope();

	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
		char *res = sdb_querys (sdb, *k, -1, NULL);
		if (res != NULL) {
			Local<String> v = NanNew(res);
			free (res);
			NanReturnValue(v);
		}
	}
	NanReturnUndefined();
}

NAN_METHOD(Database::KeysOnDisk) {
	NanScope();
	Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
	ut32 count;
	if (sdb_stats (sdb, &count, NULL)) {
		Local<Uint32> v = NanNew(count);
		NanReturnValue(v);
	}
	NanReturnUndefined();
}

NAN_METHOD(Database::KeysOnMemory) {
	NanScope();
	Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;
	ut32 count;
	if (sdb_stats (sdb, NULL, &count)) {
		Local<Uint32> v = NanNew(count);
		NanReturnValue(v);
	}
	NanReturnUndefined();
}

NAN_METHOD(Database::Expire) {
	ut32 v = 0; // should be 64 bit!
	Sdb *sdb = ObjectWrap::Unwrap<Database>(args.This())->obj;

	NanScope();

	switch (args.Length()) {
	case 1:
		if (args[0]->IsString()) {
			NanUtf8String k (args[0]);
			v = sdb_expire_get (sdb, *k, NULL);
		} else {
			NanThrowTypeError ("First argument must be a string");
		}
		break;
	case 2:
		if (args[0]->IsString()) {
			NanUtf8String k (args[0]);
			if (args[1]->IsNumber()) {
				ut64 n = (ut64)args[1]->Uint32Value();
				v = sdb_expire_set (sdb, *k, n, 0);
			} else {
				NanThrowTypeError ("2nd arg expects a number");
			}
		} else {
			NanThrowTypeError ("First argument must be a string");
		}
		break;
	default:
		NanThrowTypeError ("Missing parameters");
		break;
	}
	NanReturnValue(NanNew(v));
}

void Database::Init(Handle<Object> exports) {
	Local<v8::String> name = NanNew("Database");
	Local<v8::FunctionTemplate> ft = NanNew<FunctionTemplate>(New);
	ft->SetClassName(name);
	ft->InstanceTemplate()->SetInternalFieldCount(1);
#if 0
	auto data = Handle<Value>();
	auto ift = ft->InstanceTemplate();
	auto signature = AccessorSignature::New(isolate, ft);
	ift->SetAccessor(String::NewFromUtf8(isolate, "id"),
			GetId, 0, data, DEFAULT, ReadOnly, signature);
#endif
	NODE_SET_PROTOTYPE_METHOD(ft, "add", Add);
	NODE_SET_PROTOTYPE_METHOD(ft, "set", Set);
	//NODE_SET_PROTOTYPE_METHOD(ft, "get_length", GetLength);
	NODE_SET_PROTOTYPE_METHOD(ft, "unset", UnSet);
	NODE_SET_PROTOTYPE_METHOD(ft, "unset_like", UnSetLike);
	NODE_SET_PROTOTYPE_METHOD(ft, "reset", Reset);
	NODE_SET_PROTOTYPE_METHOD(ft, "keys_on_disk", KeysOnDisk);
	NODE_SET_PROTOTYPE_METHOD(ft, "keys_on_memory", KeysOnMemory);
	NODE_SET_PROTOTYPE_METHOD(ft, "drain", Drain);
	NODE_SET_PROTOTYPE_METHOD(ft, "unlink", UnLink);
	NODE_SET_PROTOTYPE_METHOD(ft, "get", Get);
	NODE_SET_PROTOTYPE_METHOD(ft, "type", Type);
	NODE_SET_PROTOTYPE_METHOD(ft, "sync", Sync);
	NODE_SET_PROTOTYPE_METHOD(ft, "expire", Expire);
	NODE_SET_PROTOTYPE_METHOD(ft, "query", Query);
	NODE_SET_PROTOTYPE_METHOD(ft, "exists", Exists);
	NODE_SET_PROTOTYPE_METHOD(ft, "ns", Ns);
	NODE_SET_PROTOTYPE_METHOD(ft, "like", Like);
	NODE_SET_PROTOTYPE_METHOD(ft, "open", Open);
	NODE_SET_PROTOTYPE_METHOD(ft, "close", Close);
	/* numeric stuff */
#if 0
	NODE_SET_PROTOTYPE_METHOD(ft, "inc", Inc);
	NODE_SET_PROTOTYPE_METHOD(ft, "dec", Dec);
	NODE_SET_PROTOTYPE_METHOD(ft, "min", Min);
	NODE_SET_PROTOTYPE_METHOD(ft, "max", Max);
#endif

#if 0
	/* ARRAY */
	var foo = db.array_get("foo", idx);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_get", ArrayGet);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_add", ArrayAdd);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_add_sorted", ArrayAddSorted);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_set", ArraySet);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_unset", ArrayUnset);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_remove", ArrayRemove);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_delete", ArrayDelete);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_contains", ArrayContains);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_size", ArraySize);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_length", ArrayLength);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_indexOf", ArrayIndexOf);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_insert", ArrayInsert);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_push", ArrayPush);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_pop", ArrayPop);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_sort", ArraySort);
	NODE_SET_PROTOTYPE_METHOD(ft, "array_sort_num", ArraySortNum);
#endif
#if 0
	/* NUM */
	num_exists (isNumber())
	num_get
	num_add
	num_set
	num_inc
	num_dec
	num_min
	bool_set
	bool_get
#endif
#if 0
	/* JSON */
	json_get
	json_num_inc
#endif
		exports->Set(name, ft->GetFunction());
	exports->Set(NanNew("version"), NanNew(SDB_VERSION));
	exports->Set(NanNew("encode"), NanNew <FunctionTemplate>(Encode)->GetFunction());
	exports->Set(NanNew("decode"), NanNew <FunctionTemplate>(Decode)->GetFunction());
	exports->Set(NanNew("typeof"), NanNew <FunctionTemplate>(TypeOf)->GetFunction());

	/* TODO: implement under the sdb.json object */
	exports->Set(NanNew("json_indent"), NanNew <FunctionTemplate>(JsonIndent)->GetFunction());
	exports->Set(NanNew("json_unindent"), NanNew <FunctionTemplate>(JsonUnindent)->GetFunction());
}

NAN_METHOD(Database::New) {
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowTypeError ("Database requires new");
	}

	Database *wrapper;
	int len = args.Length();
	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		wrapper = new Database(*k);
	} else {
		wrapper = new Database();
	}

	if (wrapper->obj == NULL) {
		NanThrowTypeError ("sdb_new0 returns null");
	}
	Local<v8::Object> obj = args.This();
	wrapper->Wrap(obj);
	NanReturnValue(obj);
}

#if 0
NAN_METHOD(NewDatabase) {
	Database *db = new Database();
	Local<Value> v = obj.CallAsConstructor(0, Handle<Value>[]);
	NanReturnValue(v);
}
#endif

void Init(Handle<Object> exports, Handle<Value> module) {
#if 0
	module.As<Object>()->Set(NanNew("exports"),
			NanNew <FunctionTemplate>(NewDatabase)->GetFunction());
#endif
	Database::Init(exports);
}

NODE_MODULE(sdb, Init)
