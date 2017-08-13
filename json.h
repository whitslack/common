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
	virtual Object & as_object();
	virtual const Object & as_object() const;

	virtual Array & as_array();
	virtual const Array & as_array() const;

	virtual Number & as_number();
	virtual const Number & as_number() const;

	virtual Integer & as_integer();
	virtual const Integer & as_integer() const;

	virtual String & as_string();
	virtual const String & as_string() const;

	virtual Boolean & as_boolean();
	virtual const Boolean & as_boolean() const;

protected:
	virtual std::ostream & format(std::ostream &) const = 0;

};


class Object : public Value {

public:
	typedef std::map<std::string, ValuePtr> map_t;

private:
	map_t map;

public:
	operator map_t * () { return &map; }
	operator const map_t * () const { return &map; }
	map_t * operator -> () { return &map; }
	const map_t * operator -> () const { return &map; }

	const Value * find(const std::string &key) const _pure;
	const Value & get(const std::string &key) const _pure;

	std::pair<map_t::iterator, bool> insert(const std::string &key, std::nullptr_t) {
		return map.emplace(key, nullptr);
	}
	template <typename V>
	std::pair<map_t::iterator, bool> insert(const std::string &key, V &&value) {
		return map.emplace(key, std::make_unique<V>(std::forward<V>(value)));
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
	operator vector_t * () { return &vector; }
	operator const vector_t * () const { return &vector; }
	vector_t * operator -> () { return &vector; }
	const vector_t * operator -> () const { return &vector; }

	void insert(std::nullptr_t) {
		vector.emplace_back(nullptr);
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
	virtual operator double () const = 0;

	Number & as_number() override final _const;
	const Number & as_number() const override final _const;

};


class Integer : public Number {

private:
	intmax_t value;

public:
	Integer() : value() { }
	Integer(intmax_t value) : value(value) { }

	operator intmax_t * () { return &value; }
	operator const intmax_t * () const { return &value; }
	operator double () const override { return static_cast<double>(value); }

	Integer & as_integer() override final _const;
	const Integer & as_integer() const override final _const;

protected:
	std::ostream & format(std::ostream &) const override;

};


class Real : public Number {

private:
	double value;

public:
	Real() : value() { }
	Real(double value) : value(value) { }

	operator double * () { return &value; }
	operator const double * () const { return &value; }
	operator double () const override { return value; }

protected:
	std::ostream & format(std::ostream &) const override;

};


class String : public Value {

private:
	std::string string;

public:
	String() { }
	String(const std::string &string) : string(string) { }
	String(std::string &&string) : string(std::move(string)) { }

	operator std::string * () { return &string; }
	operator const std::string * () const { return &string; }
	std::string * operator -> () { return &string; }
	const std::string * operator -> () const { return &string; }

	String & as_string() override final _const;
	const String & as_string() const override final _const;

protected:
	std::ostream & format(std::ostream &) const override;

};


class Boolean : public Value {

private:
	bool value;

public:
	Boolean() : value() { }
	Boolean(bool value) : value(value) { }

	operator bool * () { return &value; }
	operator const bool * () const { return &value; }

	Boolean & as_boolean() override final _const;
	const Boolean & as_boolean() const override final _const;

protected:
	std::ostream & format(std::ostream &) const override;

};


std::istream & operator >> (std::istream &, ValuePtr &);

std::ostream & operator << (std::ostream &, const Value &);

std::ostream & operator << (std::ostream &, const ValuePtr &);

} // namespace json
