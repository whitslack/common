#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "compiler.h"

namespace json {

class Value;
class Object;
class Array;
class Number;
class Integer;
class Real;
class String;
class Boolean;

typedef std::unique_ptr<Value> ValuePtr;


class Value {
	friend std::ostream & operator << (std::ostream &, const Value &);

public:
	virtual ~Value() = default;

public:
	_noreturn virtual Object & as_object();
	_noreturn virtual const Object & as_object() const;

	_noreturn virtual Array & as_array();
	_noreturn virtual const Array & as_array() const;

	_noreturn virtual Number & as_number();
	_noreturn virtual const Number & as_number() const;

	_noreturn virtual Integer & as_integer();
	_noreturn virtual const Integer & as_integer() const;

	_noreturn virtual String & as_string();
	_noreturn virtual const String & as_string() const;

	_noreturn virtual Boolean & as_boolean();
	_noreturn virtual const Boolean & as_boolean() const;

protected:
	virtual std::ostream & format(std::ostream &) const = 0;

};


class Object : public Value {

public:
	typedef std::map<std::string, ValuePtr> map_t;

private:
	map_t map;

public:
	_pure operator map_t * () noexcept { return &map; }
	_pure operator const map_t * () const noexcept { return &map; }
	map_t * _pure operator -> () noexcept { return &map; }
	const map_t * _pure operator -> () const noexcept { return &map; }

	const Value * find(const std::string &key) const _pure;
	const Value & get(const std::string &key) const _pure;

	std::pair<map_t::iterator, bool> insert(std::string key, std::nullptr_t) {
		return map.emplace(std::move(key), nullptr);
	}
	std::pair<map_t::iterator, bool> insert(std::string key, ValuePtr value) {
		return map.emplace(std::move(key), std::move(value));
	}
	template <typename V>
	std::pair<map_t::iterator, bool> insert(std::string key, V &&value) {
		return map.emplace(std::move(key), std::make_unique<V>(std::forward<V>(value)));
	}

	Object & as_object() override final _const;
	const Object & as_object() const override final _const;

protected:
	std::ostream & format(std::ostream &) const override;

};


class Array : public Value {

public:
	typedef std::vector<ValuePtr> vector_t;

private:
	vector_t vector;

public:
	_pure operator vector_t * () noexcept { return &vector; }
	_pure operator const vector_t * () const noexcept { return &vector; }
	vector_t * _pure operator -> () noexcept { return &vector; }
	const vector_t * _pure operator -> () const noexcept { return &vector; }

	void insert(std::nullptr_t) {
		vector.emplace_back(nullptr);
	}
	void insert(ValuePtr value) {
		vector.emplace_back(std::move(value));
	}
	template <typename V>
	void insert(V &&value) {
		vector.emplace_back(std::make_unique<V>(std::forward<V>(value)));
	}

	Array & as_array() override final _const;
	const Array & as_array() const override final _const;

protected:
	std::ostream & format(std::ostream &) const override;

};


class Number : public Value {

public:
	virtual operator double () const noexcept _pure = 0;

	Number & as_number() override final _const;
	const Number & as_number() const override final _const;

};


class Integer : public Number {

private:
	intmax_t value;

public:
	Integer(intmax_t value = 0) noexcept : value(value) { }

	_pure operator intmax_t * () noexcept { return &value; }
	_pure operator const intmax_t * () const noexcept { return &value; }
	_pure operator double () const noexcept override { return static_cast<double>(value); }

	Integer & as_integer() override final _const;
	const Integer & as_integer() const override final _const;

protected:
	std::ostream & format(std::ostream &) const override;

};


class Real : public Number {

private:
	double value;

public:
	Real(double value = 0) noexcept : value(value) { }

	_pure operator double * () noexcept { return &value; }
	_pure operator const double * () const noexcept { return &value; }
	_pure operator double () const noexcept override { return value; }

protected:
	std::ostream & format(std::ostream &) const override;

};


class String : public Value {

private:
	std::string string;

public:
	String() = default;
	String(std::string string) noexcept : string(std::move(string)) { }

	_pure operator std::string * () noexcept { return &string; }
	_pure operator const std::string * () const noexcept { return &string; }
	std::string * _pure operator -> () noexcept { return &string; }
	const std::string * _pure operator -> () const noexcept { return &string; }

	String & as_string() override final _const;
	const String & as_string() const override final _const;

protected:
	std::ostream & format(std::ostream &) const override;

};


class Boolean : public Value {

private:
	bool value;

public:
	Boolean(bool value = false) noexcept : value(value) { }

	_pure operator bool * () noexcept { return &value; }
	_pure operator const bool * () const noexcept { return &value; }

	Boolean & as_boolean() override final _const;
	const Boolean & as_boolean() const override final _const;

protected:
	std::ostream & format(std::ostream &) const override;

};


std::istream & operator >> (std::istream &, ValuePtr &);

std::ostream & operator << (std::ostream &, const Value &);

std::ostream & operator << (std::ostream &, const ValuePtr &);

} // namespace json
