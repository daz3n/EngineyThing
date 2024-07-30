#include "scripting.h"


#define INSTRUCTION3(code,arg1,arg2,arg3) { printf("\t\t%5u %5u %5u %5u", code, arg1, arg2, arg3); \
out->_code._bytecode.push_back(code); 															   \
out->_code._bytecode.push_back(arg1);															   \
out->_code._bytecode.push_back(arg2); 															   \
out->_code._bytecode.push_back(arg3); 															   \
 }									



inline bool is_integer(std::shared_ptr<object> obj)
{
	auto T = obj->_resolved_type->alias_for;

	return
		T == primitive_int8 ||
		T == primitive_int16 ||
		T == primitive_int32 ||
		T == primitive_int64 ||
		false;
}
inline bool is_floating(std::shared_ptr<object> obj)
{
	auto T = obj->_resolved_type->alias_for;

	return
		T == primitive_float32 ||
		T == primitive_float64 ||
		false;
}
inline bool find_object_or_function(v2::scope* search_scope, std::string name, object*& out_obj, function*& out_func)
{
	out_obj = nullptr;
	out_func = nullptr;



	out_obj = search_scope->find_object(name);
	if (out_obj) return true;

	out_func = search_scope->find_function(name);
	if (out_func) return true;

	if (search_scope->parent)
	{
		return find_object_or_function(search_scope->parent.get(), name, out_obj, out_func);
	}
	return false;
}






uint64_t function::create_temporary(uint64_t Size)
{
	uint64_t arg_size = 0;
	for (auto& x : _args) arg_size += x._resolved_type->stack_size;

	// also + arg size
	uint64_t ret = arg_size + _stack_size + _work_size;

	_work_size += Size;

	return ret;
}
uint64_t function::create_temporary(v2::scope* Type)
{
	Temporary Temp;
	Temp._type = Type;
	Temp._local_address = create_temporary(Type->stack_size);
	_temporaries.push_back(Temp);
	return Temp._local_address;

}

bool expression::compile_into(function* out, v2::scope* containing_scope)
{
	if (parent == nullptr || parent->operation == ")")
	{
		print2();
	}


	if (lhs) lhs->compile_into(out, containing_scope);
	if (rhs) rhs->compile_into(out, containing_scope);

	if (operation == "return")
	{
		if (rhs)
		{
			uint64_t operation;
			uint64_t arg1;
			uint64_t arg2;
			uint64_t arg3;
			if (rhs->result->_storage == storage_static)
			{
				// return global into buffer. 
				operation = opcode_return_g;
				arg1 = rhs->result->_resolved_type->stack_size;
				arg2 = rhs->result->_global_address;
			}
			if (rhs->result->_storage == storage_dynamic)
			{
				// return local into buffer
				operation = opcode_return_l;
				arg1 = rhs->result->_resolved_type->stack_size;
				arg2 = rhs->result->_local_address;
			}
			if (rhs->result->_storage == storage_none)
			{
				float as_float = std::stof(rhs->operation);
				int as_int = std::stoi(rhs->operation);

				uint64_t _arg2 = is_integer(rhs->result) ? *reinterpret_cast<uint64_t*>(&as_int) : *reinterpret_cast<uint64_t*>(&as_float);

				// return a constant into buffer. the buffer has already been specified by the last opcode_call
				operation = opcode_return_l;
				arg1 = rhs->result->_resolved_type->stack_size;
				arg2 = _arg2;
			}

			printf("%5u %5u %5u\n", operation, arg1, arg2);
			out->_code._bytecode.push_back(operation);
			out->_code._bytecode.push_back(arg1);
			out->_code._bytecode.push_back(arg2);
		}
		else
		{
			printf("%5u\n", opcode_return);
			out->_code._bytecode.push_back(opcode_return);
		}
	}
	else if (is_identifier(operation))
	{
		object* Obj = out->find_object(operation);
		function* Func = nullptr;


		// this searches outside function scope.
		// anything declared inside the function wont be a function.
		if (Obj == nullptr)
		{
			find_object_or_function((v2::scope*)out->containing_scope, operation, Obj, Func);
		}

		if (Obj)
		{
			// find object
			this->result->_name = Obj->_name;
			this->result->containing_scope = (v2::scope*)Obj->containing_scope;
			this->result->_global_address = Obj->_global_address;
			this->result->_local_address = Obj->_local_address;
			this->result->_layout_address = Obj->_layout_address;

			this->result->_resolved_type = Obj->_resolved_type;
			this->result->_storage = Obj->_storage;
			this->result->_type = Obj->_type;
			return true;
		}
		else if (Func)
		{
			// function call! 
			uint64_t value_return_address = out->create_temporary(Func->_resolved_return_type);
			printf("\t\t\tcreating temporary: address %u\n", value_return_address);

			uint64_t argssize = 0;
			for (auto& x : Func->_args) argssize += x._resolved_type->stack_size;

			// each arg is already on the stack.
			// x,y,z
			// push_value(x)
			// push_value(y)
			// push_value(z)

			// call the function
			// this saves the current stack pointer and instruction pointer in a separate list
			// the next return command restores these.
			printf("\t\t%5u %5u %5u %5u\n", opcode_call, Func->_global_address, value_return_address, argssize);
			out->_code._bytecode.push_back(opcode_call);
			out->_code._bytecode.push_back(Func->_global_address);
			out->_code._bytecode.push_back(value_return_address);
			out->_code._bytecode.push_back(argssize);


			// pop args from the stack
			printf("\t\t%5u %5u\n",opcode_pop_value, argssize);
			out->_code._bytecode.push_back(opcode_pop_value);
			out->_code._bytecode.push_back(argssize);

			// find object
			this->result->_name = Func->_name;
			this->result->containing_scope = (v2::scope*)Func->containing_scope;
			this->result->_global_address = 0;
			this->result->_local_address = value_return_address;
			this->result->_layout_address = 0;

			this->result->_resolved_type = Func->_resolved_return_type;
			this->result->_storage = storage_dynamic;
			this->result->_type = Func->_return_type;
		}
	}
	else if (std::isalnum(operation[0]) && not std::isalpha(operation[0])) // number
	{
		this->result->_name = "unnamed";
		this->result->containing_scope = (v2::scope*)out->containing_scope;
		this->result->_global_address = 0;
		this->result->_local_address = 0;
		this->result->_layout_address = 0;

		this->result->_resolved_type = this->result->containing_scope->find_primitive_type(primitive_float32).get();
		this->result->_storage = storage_none;
		this->result->_type = "<primitive-int64>";

		return true;
	}



	else if (operation == "(" || operation == ")")
	{
		printf("COMPILATION ERROR: failed to remove parenthesis from AST.\n");
		return false;
	}


	if (lhs == nullptr) return false;
	if (rhs == nullptr) return false;

	// test if both are intrinsic
	auto Lhs = lhs->result;
	auto Rhs = rhs->result;

	auto T1 = Lhs->_resolved_type;
	auto T2 = Rhs->_resolved_type;

	auto& bytecode = out->_code._bytecode;

	bool lhs_is_primitive = T1 && T1->alias_for != primitive_none;
	bool rhs_is_primitive = T2 && T2->alias_for != primitive_none;

	// member access
	if (operation == ".")
	{
		// i will have to actually read strings for this.
		// i know lhs type.
		// i dont know anything about rhs except for its name.
		// i need to search lhs.type for rhs.name to apply an offset to the base address of lhs.

		auto Member = Lhs->_resolved_type->find_type_member(rhs->operation);
		
		// storage of lhs
		// type of rhs
		// scope is func scope
		// ...

		result->_storage = Lhs->_storage;
		result->_type = Member->_type;
		result->containing_scope = Lhs->containing_scope;
		result->_name = Lhs->_name + "." + Rhs->_name;
		result->_resolved_type = Member->_resolved_type;


		// rhs must have layout storage
		if (Member->_storage != storage_layout)
		{
			printf("invalid operation '.' : rhs '%s' must be a member of '%s'\n", rhs->operation.c_str(), Lhs->_type.c_str());
		}

		if (Lhs->_storage == storage_dynamic)
		{
			result->_local_address = Lhs->_local_address + Member->_layout_address;
		}
		else if (Lhs->_storage == storage_static)
		{
			result->_local_address = Lhs->_global_address + Member->_layout_address;
		}
		else
		{
			printf("invalid operation '.' between '%s' and '%s'", Lhs->_name.c_str(), Rhs->_name.c_str());
		}
		return true;
	}

	if (operation == ",")
	{
		uint64_t operation = 0;
		uint64_t arg1;
		uint64_t arg2;

		
		// i want to have a method to evaluate the stack to see what a function is getting called with.
		// this can be done by tagging each allocation with its type.
		// 
		// these are only the push/pop instructions.
		// since every other allocation is already tagged as an object.
		
		if (lhs && lhs->result->_storage == storage_static)
		{
			operation = opcode_push_value_g;
			arg1 = lhs->result->_resolved_type->stack_size;
			arg2 = lhs->result->_global_address;
		}
		else if (lhs && lhs->result->_storage == storage_dynamic)
		{
			operation = opcode_push_value_l;
			arg1 = lhs->result->_resolved_type->stack_size;
			arg2 = lhs->result->_local_address;
		}
		else if (lhs && lhs->result->_storage == storage_none && lhs->operation != ",") // only a comma is allowed here
		{
			operation = opcode_push_value_c;
			arg1 = T1 && T1->stack_size;
			arg2 = std::stoi(lhs->operation);
		}
		if (lhs && operation)
		{
			printf("\t\t%5u %5u %5u\n", operation, arg1, arg2);
			out->_code._bytecode.push_back(operation);
			out->_code._bytecode.push_back(arg1);
			out->_code._bytecode.push_back(arg2);
		}

		
		if (rhs && rhs->result->_storage == storage_static)
		{
			operation = opcode_push_value_g;
			arg1 = rhs->result->_resolved_type->stack_size;
			arg2 = rhs->result->_global_address;
		}
		if (rhs && rhs->result->_storage == storage_dynamic)
		{
			operation = opcode_push_value_l;
			arg1 = rhs->result->_resolved_type->stack_size;
			arg2 = rhs->result->_local_address;
		}
		if (rhs && rhs->result->_storage == storage_none && rhs->operation != ",")
		{
			operation = opcode_push_value_c;
			arg1 = T2 && T2->stack_size;
			arg2 = std::stoi(rhs->operation);;
		}
		if (rhs)
		{
			printf("\t\t%5u %5u %5u\n", operation, arg1, arg2);
			out->_code._bytecode.push_back(operation);
			out->_code._bytecode.push_back(arg1);
			out->_code._bytecode.push_back(arg2);
		}


	}
	
	
	if (operation == "+" || operation == "-" || operation == "*" || operation == "/")
	{
		if (lhs_is_primitive && rhs_is_primitive)
		{
			// get max size
			auto Size1 = T1->stack_size;
			auto Size2 = T2->stack_size;
			auto BufSiz = std::max(Size1, Size2);

			// create a buffer to store the result
			auto Address = out->create_temporary(Size1 >= Size2 ? T1 : T2);
			printf("\t\t\tcreating temporary: address %u\n", Address);


			this->result->_local_address = Address;
			this->result->_type = Lhs->_type;
			this->result->_storage = storage_dynamic;
			this->result->_name = "<unnamed-temporary:" + this->result->_type + ">";
			this->result->containing_scope = (v2::scope*)out->containing_scope;
			this->result->_resolved_type = Lhs->_resolved_type;

			uint64_t operation = 0;
			uint64_t arg1 = Address;
			uint64_t arg2 = 0;
			uint64_t arg3 = 0;
			bool noop = false;

			if (is_integer(Lhs) && is_integer(Rhs))
			{
				if (Lhs->_storage == storage_static && Rhs->_storage == storage_static)
				{
					operation =
						(this->operation == "+") ? opcode_add_int_lgg :
						(this->operation == "-") ? opcode_sub_int_lgg :
						(this->operation == "*") ? opcode_mul_int_lgg :
						(this->operation == "/") ? opcode_div_int_lgg :
						0;

					arg2 = Lhs->_global_address;
					arg3 = Rhs->_global_address;
				}
				if (Lhs->_storage == storage_static && Rhs->_storage == storage_dynamic)
				{
					operation =
						(this->operation == "+") ? opcode_add_int_lgl :
						(this->operation == "-") ? opcode_sub_int_lgl :
						(this->operation == "*") ? opcode_mul_int_lgl :
						(this->operation == "/") ? opcode_div_int_lgl :
						0;

					arg2 = Lhs->_global_address;
					arg3 = Rhs->_local_address;
				}
				if (Lhs->_storage == storage_static && Rhs->_storage == storage_none)
				{
					operation =
						(this->operation == "+") ? opcode_add_int_lgc :
						(this->operation == "-") ? opcode_sub_int_lgc :
						(this->operation == "*") ? opcode_mul_int_lgc :
						(this->operation == "/") ? opcode_div_int_lgc :
						0;

					arg2 = Lhs->_global_address;
					arg3 = std::stoi(rhs->operation);
				}
				
				
				if (Lhs->_storage == storage_dynamic && Rhs->_storage == storage_static)
				{
					operation =
						(this->operation == "+") ? opcode_add_int_llg :
						(this->operation == "-") ? opcode_sub_int_llg :
						(this->operation == "*") ? opcode_mul_int_llg :
						(this->operation == "/") ? opcode_div_int_llg :
						0;

					arg2 = Lhs->_local_address;
					arg3 = Rhs->_global_address;
				}
				if (Lhs->_storage == storage_dynamic && Rhs->_storage == storage_dynamic)
				{
					operation =
						(this->operation == "+") ? opcode_add_int_lll :
						(this->operation == "-") ? opcode_sub_int_lll :
						(this->operation == "*") ? opcode_mul_int_lll :
						(this->operation == "/") ? opcode_div_int_lll :
						0;


					arg2 = Lhs->_local_address;
					arg3 = Rhs->_local_address;
				}
				if (Lhs->_storage == storage_dynamic && Rhs->_storage == storage_none)
				{
					operation =
						(this->operation == "+") ? opcode_add_int_llc :
						(this->operation == "-") ? opcode_sub_int_llc :
						(this->operation == "*") ? opcode_mul_int_llc :
						(this->operation == "/") ? opcode_div_int_llc :
						0;

					arg2 = Lhs->_local_address;
					arg3 = std::stoi(rhs->operation);
				}
				
				
				if (Lhs->_storage == storage_none && Rhs->_storage == storage_static)
				{
					operation =
						(this->operation == "+") ? opcode_add_int_lcg :
						(this->operation == "-") ? opcode_sub_int_lcg :
						(this->operation == "*") ? opcode_mul_int_lcg :
						(this->operation == "/") ? opcode_div_int_lcg :
						0;

					arg2 = std::stoi(lhs->operation);
					arg3 = Rhs->_global_address;
				}
				if (Lhs->_storage == storage_none && Rhs->_storage == storage_dynamic)
				{
					operation =
						(this->operation == "+") ? opcode_add_int_lcl :
						(this->operation == "-") ? opcode_sub_int_lcl :
						(this->operation == "*") ? opcode_mul_int_lcl :
						(this->operation == "/") ? opcode_div_int_lcl :
						0;

					arg2 = std::stoi(lhs->operation);
					arg3 = Rhs->_local_address;
				}
				if (Lhs->_storage == storage_none && Rhs->_storage == storage_none)
				{
					// calculate it here just
					if (this->operation == "+") this->operation = std::to_string(std::stoi(lhs->operation) + std::stoi(rhs->operation));
					if (this->operation == "-") this->operation = std::to_string(std::stoi(lhs->operation) - std::stoi(rhs->operation));
					if (this->operation == "*") this->operation = std::to_string(std::stoi(lhs->operation) * std::stoi(rhs->operation));
					if (this->operation == "/") this->operation = std::to_string(std::stoi(lhs->operation) / std::stoi(rhs->operation));


					this->result->_local_address = 0;
					this->result->_type = Lhs->_type;
					this->result->_storage = storage_none;
					this->result->_name = "<unnamed-temporary>";
					this->result->containing_scope = (v2::scope*)out->containing_scope;
					this->result->_resolved_type = Lhs->_resolved_type;


					noop = true;
				}

				
				// i also need to set my result
				if (not noop)
				{
					bytecode.push_back(operation);
					bytecode.push_back(arg1);
					bytecode.push_back(arg2);
					bytecode.push_back(arg3);

					printf("\t\t%5u %5u %5u %5u\n", operation, arg1, arg2, arg3);
				}
				return true;
			}
			else if (is_floating(Lhs) && is_floating(Rhs))
			{
				if (Lhs->_storage == storage_static && Rhs->_storage == storage_static)
				{
					operation =
						(this->operation == "+") ? opcode_add_float_lgg :
						(this->operation == "-") ? opcode_sub_float_lgg :
						(this->operation == "*") ? opcode_mul_float_lgg :
						(this->operation == "/") ? opcode_div_float_lgg :
						0;

					arg2 = Lhs->_global_address;
					arg3 = Rhs->_global_address;
				}
				if (Lhs->_storage == storage_static && Rhs->_storage == storage_dynamic)
				{
					operation =
						(this->operation == "+") ? opcode_add_float_lgl :
						(this->operation == "-") ? opcode_sub_float_lgl :
						(this->operation == "*") ? opcode_mul_float_lgl :
						(this->operation == "/") ? opcode_div_float_lgl :
						0;

					arg2 = Lhs->_global_address;
					arg3 = Rhs->_local_address;
				}
				if (Lhs->_storage == storage_static && Rhs->_storage == storage_none)
				{
					operation =
						(this->operation == "+") ? opcode_add_float_lgc :
						(this->operation == "-") ? opcode_sub_float_lgc :
						(this->operation == "*") ? opcode_mul_float_lgc :
						(this->operation == "/") ? opcode_div_float_lgc :
						0;

					arg2 = Lhs->_global_address;
					arg3 = std::stof(rhs->operation);
				}


				if (Lhs->_storage == storage_dynamic && Rhs->_storage == storage_static)
				{
					operation =
						(this->operation == "+") ? opcode_add_float_llg :
						(this->operation == "-") ? opcode_sub_float_llg :
						(this->operation == "*") ? opcode_mul_float_llg :
						(this->operation == "/") ? opcode_div_float_llg :
						0;

					arg2 = Lhs->_local_address;
					arg3 = Rhs->_global_address;
				}
				if (Lhs->_storage == storage_dynamic && Rhs->_storage == storage_dynamic)
				{
					operation =
						(this->operation == "+") ? opcode_add_float_lll :
						(this->operation == "-") ? opcode_sub_float_lll :
						(this->operation == "*") ? opcode_mul_float_lll :
						(this->operation == "/") ? opcode_div_float_lll :
						0;


					arg2 = Lhs->_local_address;
					arg3 = Rhs->_local_address;
				}
				if (Lhs->_storage == storage_dynamic && Rhs->_storage == storage_none)
				{
					operation =
						(this->operation == "+") ? opcode_add_float_llc :
						(this->operation == "-") ? opcode_sub_float_llc :
						(this->operation == "*") ? opcode_mul_float_llc :
						(this->operation == "/") ? opcode_div_float_llc :
						0;

					arg2 = Lhs->_local_address;
					arg3 = std::stoi(rhs->operation);
				}


				if (Lhs->_storage == storage_none && Rhs->_storage == storage_static)
				{
					operation =
						(this->operation == "+") ? opcode_add_float_lcg :
						(this->operation == "-") ? opcode_sub_float_lcg :
						(this->operation == "*") ? opcode_mul_float_lcg :
						(this->operation == "/") ? opcode_div_float_lcg :
						0;

					arg2 = std::stoi(lhs->operation);
					arg3 = Rhs->_global_address;
				}
				if (Lhs->_storage == storage_none && Rhs->_storage == storage_dynamic)
				{
					operation =
						(this->operation == "+") ? opcode_add_float_lcl :
						(this->operation == "-") ? opcode_sub_float_lcl :
						(this->operation == "*") ? opcode_mul_float_lcl :
						(this->operation == "/") ? opcode_div_float_lcl :
						0;

					arg2 = std::stoi(lhs->operation);
					arg3 = Rhs->_local_address;
				}
				if (Lhs->_storage == storage_none && Rhs->_storage == storage_none)
				{
					// calculate it here just
					if (this->operation == "+") this->operation = std::to_string(std::stof(lhs->operation) + std::stof(rhs->operation));
					if (this->operation == "-") this->operation = std::to_string(std::stof(lhs->operation) - std::stof(rhs->operation));
					if (this->operation == "*") this->operation = std::to_string(std::stof(lhs->operation) * std::stof(rhs->operation));
					if (this->operation == "/") this->operation = std::to_string(std::stof(lhs->operation) / std::stof(rhs->operation));

					this->result->_local_address = 0;
					this->result->_type = Lhs->_type;
					this->result->_storage = storage_none;
					this->result->_name = "<unnamed-temporary>";
					this->result->containing_scope = (v2::scope*)out->containing_scope;
					this->result->_resolved_type = Lhs->_resolved_type;

					noop = true;
				}


				// i also need to set my result
				if (not noop)
				{
					bytecode.push_back(operation);
					bytecode.push_back(arg1);
					bytecode.push_back(arg2);
					bytecode.push_back(arg3);

					printf("\t\t%5u %5u %5u %5u\n", operation, arg1, arg2, arg3);
				}
				return true;
			}
			else
			{
				printf("failed to compile expression (%s %s %s): primitive types dont match (%s != %s)\n",
					Lhs->_name.c_str(),
					this->operation.c_str(),
					Rhs->_name.c_str(),
					Lhs->_resolved_type->name.c_str(), Rhs->_resolved_type->name.c_str());
			}
		}
		else
		{
			// call function
			// call_function();
			printf("1: ERROR IN COMPILATION: '%s' '%s' '%s' - all operands must be a primitive type\n",
				T1 ? T1->name.c_str() : "null",
				operation.c_str(),
				T2 ? T2->name.c_str() : "null"
			);

		}
	}

	if (operation == "=")
	{
		if (lhs_is_primitive && rhs_is_primitive)
		{
			// get max size
			auto Size1 = T1->stack_size;
			auto Size2 = T2->stack_size;
			auto BufSiz = std::max(Size1, Size2);

			this->result->_type = Lhs->_type;
			this->result->_storage = storage_dynamic;
			this->result->_name = "<unnamed-temporary:" + this->result->_type + ">";
			this->result->containing_scope = (v2::scope*)out->containing_scope;

			bool lhs_is_int = is_integer(Lhs);

			uint64_t operation = 0;
			uint64_t arg1 = 0; // lhs->address
			uint64_t arg2 = 0;
			bool noop = false;

			if (Lhs->_storage == storage_static && Rhs->_storage == storage_static)
			{
				this->result->_global_address = Lhs->_global_address;

				operation = lhs_is_int ? opcode_set_int_gg : opcode_set_float_gg;
				arg1 = Lhs->_global_address;
				arg2 = Rhs->_global_address;
			}
			if (Lhs->_storage == storage_static && Rhs->_storage == storage_dynamic)
			{
				this->result->_global_address = Lhs->_global_address;

				operation = lhs_is_int ? opcode_set_int_gl : opcode_set_float_gl;
				arg1 = Lhs->_global_address;
				arg2 = Rhs->_local_address;
			}
			if (Lhs->_storage == storage_static && Rhs->_storage == storage_none)
			{
				this->result->_global_address = Lhs->_global_address;

				operation = lhs_is_int ? opcode_set_int_gc : opcode_set_float_gc;
				arg1 = Lhs->_global_address;
				arg2 = std::stoi(rhs->operation);
			}

			

			if (Lhs->_storage == storage_dynamic && Rhs->_storage == storage_static)
			{
				this->result->_local_address = Lhs->_local_address;

				operation = lhs_is_int ? opcode_set_int_lg : opcode_set_float_lg;
				arg1 = Lhs->_local_address;
				arg2 = Rhs->_global_address;
			}
			if (Lhs->_storage == storage_dynamic && Rhs->_storage == storage_dynamic)
			{
				this->result->_local_address = Lhs->_local_address;

				operation = lhs_is_int ? opcode_set_int_ll : opcode_set_float_ll;
				arg1 = Lhs->_local_address;
				arg2 = Rhs->_local_address;
			}
			if (Lhs->_storage == storage_dynamic && Rhs->_storage == storage_none)
			{
				this->result->_local_address = Lhs->_local_address;

				operation = lhs_is_int ? opcode_set_int_lc : opcode_set_float_lc;
				arg1 = Lhs->_local_address;
				arg2 = std::stoi(rhs->operation);
			}
			

			printf("\t\t%5u %5u %5u\n", operation, arg1, arg2);
			bytecode.push_back(operation);
			bytecode.push_back(arg1);
			bytecode.push_back(arg2);
		}
		else
		{
			// call function
			// call_function();
		
			if (T1 == T2)
			{
				// just copy bytes from src to dst
				uint64_t operation;
				uint64_t arg1;
				uint64_t arg2;
				uint64_t arg3;

				if (lhs->result->_storage == storage_static && rhs->result->_storage == storage_static)
				{
					operation = opcode_copy_gg;
					arg1 = lhs->result->_global_address;
					arg2 = rhs->result->_global_address;
					arg3 = lhs->result->_resolved_type->stack_size;
				}
				
				if (lhs->result->_storage == storage_static && rhs->result->_storage == storage_dynamic)
				{
					operation = opcode_copy_gl;
					arg1 = lhs->result->_global_address;
					arg2 = rhs->result->_local_address;
					arg3 = lhs->result->_resolved_type->stack_size;
				}
				
				if (lhs->result->_storage == storage_dynamic && rhs->result->_storage == storage_static)
				{
					operation = opcode_copy_lg;
					arg1 = lhs->result->_local_address;
					arg2 = rhs->result->_global_address;
					arg3 = lhs->result->_resolved_type->stack_size;
				}
				if (lhs->result->_storage == storage_dynamic && rhs->result->_storage == storage_dynamic)
				{
					operation = opcode_copy_ll;
					arg1 = lhs->result->_local_address;
					arg2 = rhs->result->_local_address;
					arg3 = lhs->result->_resolved_type->stack_size;
				}

				printf("\t\t%5u %5u %5u %5u\n", operation, arg1, arg2, arg3);
				out->_code._bytecode.push_back(operation);
				out->_code._bytecode.push_back(arg1);
				out->_code._bytecode.push_back(arg2);
				out->_code._bytecode.push_back(arg3);
			}

			else
			{
				printf("3: ERROR IN COMPILATION: can't assign to %s %s. all operands must match or be primitives\n",
					T1->name.c_str(),
					lhs->result->_name.c_str()
				);
				printf("trying to '%s' '%s' and '%s'\n",operation.c_str(),
					T1 ? T1->name.c_str() : "null",
					T2 ? T2->name.c_str() : "null"
				);
			}

		}
	}
}

void transform_parens_L(expression* e)
{
	// todo:
	// other paren becomes rhs of lhs

	if (e->operation == "(")
	{
		if (e->rhs && e->lhs)
		{
			if (e->lhs->rhs)
			{
				expression* add = e->lhs->rhs;
				while (add->rhs)
				{
					add = add->rhs;
				}

				add->rhs = e->rhs;
				e->rhs->parent = add;
				
				// e->lhs->rhs->rhs = e->rhs;
				// e->rhs->parent = e->lhs->rhs;
			}
			else if (e->lhs)
			{
				e->lhs->rhs = e->rhs;
				e->rhs->parent = e->lhs;
			}
			e->rhs = nullptr;
		}

		// also remove this from the tree
		if (e->parent)
		{
			e->lhs->parent = e->parent;
			
			if (e->parent->lhs == e)
			{
				e->parent->lhs = e->lhs;
			}
			if (e->parent->rhs == e)
			{
				e->parent->rhs = e->lhs;
			}
			e->parent = nullptr;
		}
	}
}


void transform_parens_R(expression* e)
{
	// todo:
	// i need to remove the root if its a parenthesis

	if (e->operation == ")")
	{
		if (e->rhs && e->lhs)
		{
			if (e->lhs)
			{
				e->rhs->lhs = e->lhs->rhs;

				e->lhs->rhs = e->rhs;
				e->rhs->parent = e->lhs;
			}

			e->rhs = nullptr;
		}

		// also remove this from the tree
		if (e->parent)
		{
			e->lhs->parent = e->parent;
			
			if (e->parent->lhs == e)
			{
				e->parent->lhs = e->lhs;
			}
			if (e->parent->rhs == e)
			{
				e->parent->rhs = e->lhs;
			}
			e->parent = nullptr;
		}
	}
}


bool statement::compile_into(function* out, v2::scope* containing_scope)
{
	// this has to be done in 2 passes.

	// this currently doesnt work for (5 + (5 * 5)). it will be 25 not 30. all the parens are needed...
	root->transform(transform_parens_L);
	root->transform(transform_parens_R);


	if (root->operation == ")")
	{
		root = root->lhs;
	}

	return root->compile_into(out, containing_scope);
}






































#if 0
#include "../utils/string/include.h"
#include "../utils/event/action.h"

#include <unordered_map>
#include <string>

struct object;

struct script_callback
{
	string name;
	action_with<object*> callback;
};
struct item
{
	string name;
	size_t value = 1;
	size_t weight = 1;
	size_t stack_size = 1;
	std::vector<script_callback> inventory_actions;
};

struct object
{
	
};


struct ScriptingEngine
{
public:
	
	item get_item(string itemname)
	{
		item ret;

		// read ../items/{itemname}.txt
		// set all item info
		// add all actions to the item, along with their name.
		return ret;
	}

	void call(object* caller, script_callback& callback)
	{
		// [recover] 50% HP
		callback.callback(caller);
	}
};

struct inventory
{
	void display(object* actor)
	{
		// get the actor inventory

		// for each item in inventory

		// get item icon
		// ../icon/{itemname}.png

		// if clicked
			// get item actions
			// if clicked
			// action(actor)
			
	}
};


#endif 