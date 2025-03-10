#include "Core/Object.h"

namespace Ry
{

	const ReflectedClass* ObjectBase::GetClass() const
	{
		return nullptr;
	}

	void Object::SetObjectName(const Ry::String& NewName)
	{
		this->ObjectName = NewName;
	}

	Ry::String Object::GetObjectName() const
	{
		return ObjectName;
	}

	Json Jsonify(Ry::Object& Object)
	{
		Json RootJson = Json::CreateObject();
		const ReflectedClass* Class = Object.GetClass();

		for(const Ry::Field& RefField : Class->Fields)
		{
			// Unsigned integral types
			if (RefField.Type->Class == TypeClass::UInt8)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<uint8>(&Object));
			if (RefField.Type->Class == TypeClass::UInt16)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<uint16>(&Object));
			if (RefField.Type->Class == TypeClass::UInt32)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<uint32>(&Object));
			if (RefField.Type->Class == TypeClass::UInt64)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<uint64>(&Object));

			// Signed integral types
			if (RefField.Type->Class == TypeClass::Int8)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<int8>(&Object));
			if (RefField.Type->Class == TypeClass::Int16)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<int16>(&Object));
			if (RefField.Type->Class == TypeClass::Int32)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<int32>(&Object));
			if (RefField.Type->Class == TypeClass::Int64)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<int64>(&Object));

			// Floating point types
			if (RefField.Type->Class == TypeClass::Float)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<float>(&Object));
			if (RefField.Type->Class == TypeClass::Double)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<double>(&Object));

			// Strings
			if (RefField.Type->Class == TypeClass::String)
				RootJson.Insert(RefField.Name, *RefField.GetConstPtrToField<Ry::String>(&Object));
		}

		return RootJson;
	}
}
