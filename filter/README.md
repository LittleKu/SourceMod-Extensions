filter
======

This is a extension for SourceMod, Only test on CS:GO, but I think it should work on the game with Source Engine

Author  --> LittleKu(K.K.Lv)

date    --> 2014/11/16

email   --> kklvzl@gmail.com

QQ      --> 116268742



/**
 * Get the count of the banned ip list 
 * 
 * @return		the count of the banned ip list
 */

native FT_GetFilterCount();

/**
 * Get a ip from the filter list with the given index
 * 
 * @param nIndex		the index at the banned ip list, should be start at 0 and the last one is count - 1.
 * @param ip			store the ip with the nIndex
 * @param maxLen		the max length store the ip
 * @return				1 on success, 0 on failed.
 */

native FT_GetFilterIP(nIndex, String:ip[], maxLen);


/**
 * Whether the ip has been banned or not.
 * 
 * @param ip			the ip want to check whether is banned or not
 * @param bantime		If the IP is banned, it will save time to be banned
 * @return				1:the ip has been banned, otherwise not
 */

native FT_GetBannedTime(const String:ip[], &Float:bantime);

/**
 * Call when type "addip" command
 * 
 * @note Ignore the return value.
 * 
 * @param cmd			always is "addip"
 * @param bantime		how long does the ip was banned
 * @param ip			the ip
 * 
 */

forward OnFillterAdd_Post(const String:cmd[], Float:bantime, const String:ip[]);
