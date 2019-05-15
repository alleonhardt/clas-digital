/**
 * @file src/login/user_system.hpp
 *
 * This file defines the interface for the basic user class
 *
 */
#include <string>
#include <map>
#ifdef COMPILE_UNITTEST
#include <gtest/gtest.h>
#endif

/**
 * @brief Defines the basic a access rights a user can have at the moment
 */
enum AccessRights
{
	USR_READ = 1,	///< The user has read access means, he can access all books for reading
	USR_WRITE = 2,	///< The user has write access he upload new books and change existing ones
	USR_ADMIN = 4	///< The user is an admin he can create new users and give all users new rights, he can delete users as well
};

/**
 * @brief The basic user class this represents a basic user and stores email password and access rights for this user
 *
 */
class User
{
	private:
		std::string _password; 		///<The user password.
		std::string _email;			///<The email of the user.
		volatile int _accessRights;	///<The access rights the user has.

	public:
		/**
		 * Default constructor for the User class.
		 */
		User();

		/**
		 * Constructs a user with a given email, password and access rights.
		 * @param email The email the user got
		 * @param pass	The password the user will use to login
		 * @param access The access rights the user got.
		 */
		User(const char *email, const char *pass, int access);


		/**
		 * Returns the user information as json file.
		 * User information means: email and access rights.
		 * @return A string in the json format containing email and access rights of the user
		 */
		std::string toJSON() const;

		/**
		 * Getter for the access rights of the user.
		 */
		int GetAccessRights() const;

		/**
		 * Setter for the access rights of the user.
		 * @param acc The new access rights for the user.
		 */
		void SetAccessRights(int acc); 

		/**
		 * The getter for the email the user uses.
		 */
		const std::string &GetEmail() const;

		/**
		 * The getter for the password of the user.
		 */
		const std::string &GetPassword() const;

		/**
		 * Checks if the given password and email matches the users credentials.
		 * @param email The email to check against
		 * @param passwd The Password to check against
		 * @return Returns true if the given credentials matches the user credentials
		 */
		bool DoesMatch(std::string email, std::string passwd) const;
};

