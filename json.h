#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "compiler.h"

namespace json {

typedef std::unique_ptr<class Value> ValuePtr;


class Value {

public:
	Value() { }
	Value(Value &&) = default;
	virtual ~Value() { }
	virtual std::ostream & format(std::ostream &) const = 0;
private:
	Value(const Value &) = delete;
	Value & operator = (const Value &) = delete;

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
	std::pair<map_t::iterator, bool> insert(const std::string &key, std::nullptr_t) {
		return map.emplace(key, nullptr);
	}
	template <typename V>
	std::pair<map_t::iterator, bool> insert(const std::string &key, V &&value) {
		return map.emplace(key, ValuePtr(new typename std::decay<V>::type(std::forward<V>(value))));
	}
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
		vector.emplace_back(new typename std::decay<V>::type(std::forward<V>(value)));
	}
	std::ostream & format(std::ostream &) const override;

};


class Number : public Value {

public:
	virtual operator double () const = 0;

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
	std::ostream & format(std::ostream &) const override;

};


std::istream & operator >> (std::istream &, ValuePtr &);

std::ostream & operator << (std::ostream &, const Value &);

std::ostream & operator << (std::ostream &, const ValuePtr &);


const Value * find(const Object &object, const std::string &key) _pure;

const Value & get(const Object &object, const std::string &key);

const Object & as_object(const Value &value);

const Array & as_array(const Value &value);

const Number & as_number(const Value &value);

const Integer & as_integer(const Value &value);

const String & as_string(const Value &value);

const Boolean & as_boolean(const Value &value);

} // namespace json
