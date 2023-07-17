#pragma once
#include <string>

class User
{
	std::string _login;
	std::string _password;
	std::string _name;
	std::string _gender;
public:
	User(const std::string& login, const std::string& password, const std::string &name):_login(login),_password(password),_name(name) {}
	User(const std::string& name, const std::string& gender) : _name(name), _gender(gender) {}
	User(const std::string& name) : _name(name) {}
	User(const std::string& login, const std::string& password, const std::string &name, const std::string &gender) :_login(login), _password(password), _name(name),_gender(gender) {}
	const std::string& getUserLogin() const { return _login;}
	const std::string& getUserPassword() const { return _password; }
	const std::string& getUserName() const { return _name; }
	const std::string& getUserGender() const { return _gender; }

	void setUserPassword(const std::string &password) { _password=password; }
	void setUserName(const std::string& name) { _name = name; }
	void setUserGender(std::string gender) { _gender=gender; }

	friend std::fstream& operator >>(std::fstream& is, User& obj);
	friend std::ostream& operator <<(std::ostream& os, const User& obj);
};
