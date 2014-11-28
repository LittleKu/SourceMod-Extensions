#if defined _dropreason_included
#endinput
#endif
#define _dropreason_included

/* Called before the client drop from the server
 * Ignore the return value
 *
 * @param slot			The slot of the client in this server. should be start at 0
 * @param name			The name of the client
 * @param reason		The reason why the player drop from the server.
 *
*/
forward OnClientDropPost(iSlot, const String:szName[], const String:szReason[]);

public Extension:__ext_dropreason =
{
	name = "dropreason",
	file = "dropreason.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};
