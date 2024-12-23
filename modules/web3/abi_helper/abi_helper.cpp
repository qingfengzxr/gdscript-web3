#include "abi_helper.h"

ABIHelper::ABIHelper() {
    m_methods.clear();
    m_events.clear();
    m_errors.clear();

    m_methods_index.clear();
    m_events_index.clear();
    m_errors_index.clear();

    m_constructor = nullptr;
    m_fallback = nullptr;
    m_receive = nullptr;
}

ABIHelper::~ABIHelper() {
	if (m_methods.size() > 0) {
		for (int i = 0; i < m_methods.size(); ++i) {
			if (m_methods[i]) {
				memdelete(m_methods.write[i]);
				m_methods.write[i] = nullptr;
			}
		}
		m_methods.clear();
		m_methods_index.clear();
	}

	if (m_events.size() > 0) {
		for (int i = 0; i < m_events.size(); ++i) {
			if (m_events[i]) {
				memdelete(m_events.write[i]);
				m_events.write[i] = nullptr;
			}
		}
		m_events.clear();
		m_events_index.clear();
	}

	if (m_errors.size() > 0) {
		for (int i = 0; i < m_errors.size(); ++i) {
			if (m_errors[i]) {
				memdelete(m_errors.write[i]);
				m_errors.write[i] = nullptr;
			}
		}
		m_errors.clear();
		m_errors_index.clear();
	}

    if (m_constructor) {
        memdelete(m_constructor);
        m_constructor = nullptr;
    }
    if (m_fallback) {
        memdelete(m_fallback);
        m_fallback = nullptr;
    }
    if (m_receive) {
        memdelete(m_receive);
        m_receive = nullptr;
    }
}

void ABIHelper::format_output() const {
    print_line("ABIHelper {");

    print_line("  methods: [");
    for (int i = 0; i < m_methods.size(); ++i) {
        m_methods[i]->format_output();
    }
    print_line("  ]");

    print_line("}");
}

PackedByteArray ABIHelper::pack(String name, const Array &args) {
	PackedByteArray arguments;
    if ( name == "" ) {
        // constructor
        Dictionary result = abiarguments_pack(m_constructor->inputs, args);
        if (int(result["error"]) != OK) {
            ERR_PRINT(vformat("abiarguments pack constructor failed, errmsg: %s", result["errmsg"]));
            return arguments;
        }
		arguments = result["packed"];
        return arguments;
    }

    ABIMethod* method = get_abimethod(name);
    ERR_FAIL_COND_V_MSG(method == nullptr , PackedByteArray(), "method " + name + " not found");

    Dictionary result = abiarguments_pack(method->inputs, args);
	if (int(result["error"])!= OK) {
		ERR_PRINT(vformat("abiarguments pack failed, errmsg: %s", result["errmsg"]));
		return arguments;
	}
	arguments = result["packed"];

    PackedByteArray res = method->id;
    res.append_array(arguments);
    return res;
}

ABIArguments ABIHelper::get_arguments(const String &name, const PackedByteArray &data) {
    ABIArguments args;
    if (m_methods_index.has(name)) {
        int index = m_methods_index[name];
        ABIMethod* method = m_methods[index];
        if (data.size() % 32 != 0) {
            ERR_PRINT(vformat("abi: improperly formatted data size: %d", data.size()));
            return args;
        }
        args = method->outputs;
        return args;
    }
    if (m_events_index.has(name)) {
        int index = m_events_index[name];
        ABIEvent* event = m_events[index];
        args = event->inputs;
        return args;
    }

    if (args.size() == 0) {
        ERR_PRINT("abi: could not locate named method or event: " + name);
        return args;
    }

    return args;
}

Error ABIHelper::unpack_into_dictionary(String name, PackedByteArray data, Dictionary result) {
    ABIArguments args = get_arguments(name, data);
    if (args.size() == 0) {
        ERR_PRINT("abi: could not locate named method or event: " + name);
        return FAILED;
    }
    Variant res = result;
    Error err = unpack_abiarguments(args, data, res);
    if (err != OK) {
        return err;
    }
    result = res;
    return OK;
}

Error ABIHelper::unpack_into_array(String name, PackedByteArray data, Array result) {
    ABIArguments args = get_arguments(name, data);
    if (args.size() == 0) {
        ERR_PRINT("abi: could not locate named method or event: " + name);
        return FAILED;
    }

    Variant res = result;
    Error err = unpack_abiarguments(args, data, res);
    if (err != OK) {
        return err;
    }
    result = res;
    return OK;
}

Vector<Field> unmarshal_json_to_fieldlist(const String &json_str) {
    JSON *json = memnew(JSON);
    Error err = json->parse(json_str);
    Vector<Field> fieldList;

    if (err == OK) {
        Variant data = json->get_data();
        if (data.get_type() == Variant::ARRAY) {
            Array fieldsArray = data;
            for (int i = 0; i < fieldsArray.size(); ++i) {
                Dictionary fieldDict = fieldsArray[i];
                Field field;
                field.unmarshal(fieldDict);
                fieldList.push_back(field);
            }
        }
    }

    return fieldList;
}

String marshal_fieldlist_to_json(const Vector<Field> &fieldList) {
    Array fieldsArray;

    for (int i = 0; i < fieldList.size(); ++i) {
        const Field &field = fieldList[i];
        Dictionary fieldDict;

        fieldDict["type"] = field.type;

		if (field.name != "") {
	        fieldDict["name"] = field.name;
		}

		if (field.state_mutability != "") {
	        fieldDict["stateMutability"] = field.state_mutability;
		}

		if (field.constant) {
	        fieldDict["constant"] = field.constant;
		}

		if (field.payable) {
	        fieldDict["payable"] = field.payable;
		}

		if (field.anonymous) {
			fieldDict["anonymous"] = field.anonymous;
		}

        Array inputsArray;
        for (int j = 0; j < field.inputs.size(); ++j) {
            const ABIArgumentMarshaling &input = field.inputs[j];
            Dictionary inputDict;
            inputDict["name"] = input.name;
            inputDict["type"] = input.type;
            inputDict["internalType"] = input.internalType;
			if (input.indexed) {
	            inputDict["indexed"] = input.indexed;
			}

            Array componentsArray;
            for (int k = 0; k < input.components.size(); ++k) {
                const ABIArgumentMarshaling &component = input.components[k];
                Dictionary componentDict;
                componentDict["name"] = component.name;
                componentDict["type"] = component.type;
                componentDict["internalType"] = component.internalType;
				if (component.indexed) {
	                componentDict["indexed"] = component.indexed;
				}
                componentsArray.push_back(componentDict);
            }

            if (!componentsArray.is_empty()) {
                inputDict["components"] = componentsArray;
            }

            inputsArray.push_back(inputDict);
        }
		fieldDict["inputs"] = inputsArray;

        Array outputsArray;
        for (int j = 0; j < field.outputs.size(); ++j) {
            const ABIArgumentMarshaling &output = field.outputs[j];
            Dictionary outputDict;
            outputDict["name"] = output.name;
            outputDict["type"] = output.type;
            outputDict["internalType"] = output.internalType;
			if (output.indexed) {
	            outputDict["indexed"] = output.indexed;
			}

            Array componentsArray;
            for (int k = 0; k < output.components.size(); ++k) {
                const ABIArgumentMarshaling &component = output.components[k];
                Dictionary componentDict;
                componentDict["name"] = component.name;
                componentDict["type"] = component.type;
                componentDict["internalType"] = component.internalType;
				if (component.indexed) {
	                componentDict["indexed"] = component.indexed;
				}

                componentsArray.push_back(componentDict);
            }

            if (!componentsArray.is_empty()) {
                outputDict["components"] = componentsArray;
            }

            outputsArray.push_back(outputDict);
        }
        if (!outputsArray.is_empty()) {
            fieldDict["outputs"] = outputsArray;
        }

        fieldsArray.push_back(fieldDict);
    }

     JSON *json = memnew(JSON);
    String jsonString = json->stringify(fieldsArray);
    return jsonString;
}

bool ABIHelper::has_fallback() {
    return m_fallback->type == Fallback;
}

bool ABIHelper::has_receive() {
    return m_receive->type == Receive;
}

bool ABIHelper::unmarshal_from_json(const String &json_str) {
    Vector<Field> fields = unmarshal_json_to_fieldlist(json_str);

    for (auto &v: fields) {
        // printf("type: %s, name: %s\n", v.type.utf8().get_data(), v.name.utf8().get_data());

        String fieldType = v.type;
        if (fieldType == "constructor") {
            m_constructor = NewABIMethod("", "", Constructor, v.state_mutability, v.constant, v.payable, v.inputs, ABIArguments());
        } else if (fieldType == "function") {
            String name = ResolveNameConflict(v.name, [&](const String &s) { return this->get_abimethod(s) != nullptr; });
            // m_methods[name] = NewABIMethod(name, v.name, Function, v.state_mutability, v.constant, v.payable, v.inputs, v.outputs);

            auto method = NewABIMethod(name, v.name, Function, v.state_mutability, v.constant, v.payable, v.inputs, v.outputs);
            this->add_abimethod(name, method);
        } else if (fieldType == "fallback") {
            ERR_FAIL_COND_V_MSG(has_fallback() , false, "only single fallback is allowed");

            m_fallback = NewABIMethod("", "", Fallback, v.state_mutability, v.constant, v.payable, ABIArguments(), ABIArguments());
        } else if (fieldType == "receive") {
            ERR_FAIL_COND_V_MSG(has_receive() , false, "only single receive is allowed");
            ERR_FAIL_COND_V_MSG(v.state_mutability != "payable" , false, "the statemutability of receive can only be payable");

            m_receive = NewABIMethod("", "", Receive, v.state_mutability, v.constant, v.payable, ABIArguments(), ABIArguments());
        } else if (fieldType == "event") {
            String name = ResolveNameConflict(v.name, [&](const String &s) { return this->get_abievent(s) != nullptr; });
            auto event = NewABIEvent(name, v.name, v.anonymous, v.inputs);
            this->add_abievent(name, event);
        } else if (fieldType == "error") {
            auto error = NewABIError(v.name, v.inputs);
            this->add_abierror(v.name, error);
        } else {
            String errmsg = "abi: could not recognize type " + v.type + " of field " + v.name.utf8().get_data();
            ERR_FAIL_COND_V_MSG(true, false, errmsg);
        }
    }

    return true;
}

void ABIHelper::add_abimethod(const String &name, ABIMethod* method) {
    m_methods.push_back(method);
    int method_index = m_methods.size() - 1;
    m_methods_index[name] = method_index;
}

ABIMethod* ABIHelper::get_abimethod(const String &name) {
    if (m_methods_index.has(name)) {
        int index = m_methods_index[name];
        return m_methods[index];
    }
    return nullptr;
}


int ABIHelper::abimethod_count() {
    return m_methods.size();
}

void ABIHelper::add_abievent(const String &name, ABIEvent* event) {
    m_events.push_back(event);
    int index = m_events.size() - 1;
    m_events_index[name] = index;
}

ABIEvent* ABIHelper::get_abievent(const String &name) {
    if (m_events_index.has(name)) {
        int index = m_events_index[name];
        return m_events[index];
    }
    return nullptr;
}

int ABIHelper::abievent_count() {
    return m_events.size();
}

void ABIHelper::add_abierror(const String &name, ABIError* error) {
    m_errors.push_back(error);
    int index = m_errors.size() - 1;
    m_errors_index[name] = index;
}

ABIError* ABIHelper::get_abierror(const String &name) {
    if (m_errors_index.has(name)) {
        int index = m_errors_index[name];
        return m_errors[index];
    }
    return nullptr;
}

int ABIHelper::abierror_count() {
    return m_errors.size();
}

void ABIHelper::_bind_methods() {
    ClassDB::bind_method(D_METHOD("unmarshal_from_json", "json_str"), &ABIHelper::unmarshal_from_json);
    ClassDB::bind_method(D_METHOD("abimethod_count"), &ABIHelper::abimethod_count);
    ClassDB::bind_method(D_METHOD("abierror_count"), &ABIHelper::abierror_count);
    ClassDB::bind_method(D_METHOD("abievent_count"), &ABIHelper::abievent_count);
	ClassDB::bind_method(D_METHOD("pack", "name", "args"), &ABIHelper::pack);
	ClassDB::bind_method(D_METHOD("unpack_into_dictionary", "name", "data", "result"), &ABIHelper::unpack_into_dictionary);
	ClassDB::bind_method(D_METHOD("unpack_into_array", "name", "data", "result"), &ABIHelper::unpack_into_array);

	ClassDB::bind_method(D_METHOD("format_output"), &ABIHelper::format_output);
}
