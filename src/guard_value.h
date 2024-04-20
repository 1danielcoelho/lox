#pragma once

template<typename T>
class ScopedGuardValue
{
public:
	ScopedGuardValue(T& in_variable, const T& new_value)
		: variable(in_variable)
	{
		old_value = variable;
		variable = new_value;
	}

	~ScopedGuardValue()
	{
		variable = old_value;
	}

	ScopedGuardValue(const ScopedGuardValue& other) = delete;
	ScopedGuardValue(ScopedGuardValue&& other) = delete;
	ScopedGuardValue& operator=(const ScopedGuardValue& other) = delete;
	ScopedGuardValue& operator=(ScopedGuardValue&& other) = delete;

private:
	T& variable;
	T old_value;
};
