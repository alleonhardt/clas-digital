/**
 * @file src/server/URIObjects.hpp 
 *
 * Defines the basic URI Objects /search and /getprofileinfo etc.
 *
 */
#pragma once
#include "src/server/BasicHandlers.hpp"

/**
 * @brief 
 */
class UserSystemHandler : public EmptyHandler
{
	public:

		/**
		 * @brief Sends back specific informations regarding the user profile and profile changes 
		 * @param headers
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};

/**
 * @brief
 */
class UpdateUserSystemHandler : public EmptyHandler
{
	public:
		void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
};

class GetBookRessource : public EmptyHandler
{
	public:
		/**
		 * @brief Sends back specific informations regarding the user profile and profile changes 
		 * @param headers
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};
