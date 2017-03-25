#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "wlconf.h"

static int add_member(struct maclist *list, char *macaddr)
{
	struct maclist_node *node = malloc(sizeof(struct maclist_node));
	strcpy(node->macaddr, macaddr);
	node->next = NULL;
	if (list->head == NULL)
	{
		list->head = node;
		list->tail = node;
	}
	else 
	{
		list->tail->next = node;
		list->tail = node;
	}
	list->listsize++;
}

static int del_member(struct maclist *list, char *macaddr)
{
	if (list->listsize == 0)
	{
		return -1;
	}
	struct maclist_node *node = list->head;
	if (!(strcmp(node->macaddr, macaddr)))
	{
		list->head = node->next;
		free(node);
		list->listsize--;
		return 0;
	}
	while (node->next != NULL)
	{
		if (!(strcmp(node->next->macaddr, macaddr)))
		{
			struct maclist_node *s_node = node->next;
			if (s_node->next == NULL)
			{
				list->tail = node;
				node->next = NULL;
				free(s_node);
				list->listsize--;
				return 0;
			}
			node->next = s_node->next;
			free(s_node);
			list->listsize--;
			return 0;
		}
		node = node->next;
	}
	printf ("macaddr not found:%s\n", macaddr);
	return -1;
}

static void free_maclist(struct maclist *list)
{
	struct maclist_node *node = list->head;
	while (node->next != NULL)
	{
		free(node);
		node = node->next;
	}
	free(node);
}

/*
 * Definition: add a member to a mac list(for filter)
 * Return:  0	- success
 * 		   -1	- failed
 */
static add_macfilterlist(struct wlconf *wlconf, char *macaddr)
{
	char str[100];
	struct uci_ptr ptr;
	int rv;
	strcpy(str, "wireless.@wifi-iface[0].maclist=");
	strcat(str, macaddr);
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("ADD_LIST_ERROR:error in uci_lookup_ptr\n");
		return -1;
	}
	rv = uci_add_list(wlconf->ctx, &ptr);
	if (rv != UCI_OK)
	{
		printf("ADD_LIST_ERROR:error in uci_add_list\n");
		return -1;
	}
	uci_save(wlconf->ctx, ptr.p);
	wlconf->conf->macfilter_list->add_member(wlconf->conf->macfilter_list, macaddr);
	return 0;
	// uci_commit(wlconf->ctx, &ptr.p, true);
}

/*
 * Definition: del a member from a mac list(for filter)
 * Return:  0	- success
 * 		   -1	- failed
 */
static del_macfilterlist(struct wlconf *wlconf, char *macaddr)
{
	char str[100];
	struct uci_ptr ptr;
	int rv;
	strcpy(str, "wireless.@wifi-iface[0].maclist=");
	strcat(str, macaddr);
	if (wlconf->conf->macfilter_list->listsize == 0)
	{
		printf("DELETE_LIST_ERROR:list empty\n");
		return -1;
	}
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("DELETE_LIST_ERROR:error in uci_lookup_ptr\n");
	}
	rv = uci_del_list(wlconf->ctx, &ptr);
	if (rv != UCI_OK)
	{
		printf("DELETE_LIST_ERROR:error in uci_del_list\n");
		return -1;
	}
	uci_save(wlconf->ctx, ptr.p);
	wlconf->conf->macfilter_list->del_member(wlconf->conf->macfilter_list, macaddr);
	// uci_commit(wlconf->ctx, &ptr.p, true);
}

static int set_ssid(struct wlconf *wlconf, char *ssid)
{
	struct uci_ptr ptr;
	int rv;
	char str[100];
	strcpy(str, "wireless.@wifi-iface[0].ssid=");
	strcat(str, ssid);
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("SET_SSID_ERROR:error in uci_lookup_ptr\n");
	}
	if (wlconf->ctx == NULL)
	{
		printf("SET_SSID_ERROR:invalid context\n");
		return -1;
	}
	rv = uci_set(wlconf->ctx, &ptr);
	if (rv != UCI_OK)
	{
		printf("SET_SSID_ERROR:error in uci_set\n");
		return -1;
	}
	uci_save(wlconf->ctx, ptr.p);
	strcpy(wlconf->conf->ssid, ssid);
	// uci_commit(wlconf->ctx, &ptr.p, true);
	return 0;
}

static int set_channel(struct wlconf *wlconf, int channel)
{
	struct uci_ptr ptr;
	int rv;
	char str[100];
	char s_channel[2];
	sprintf(s_channel, "%d", channel);
	strcpy(str, "wireless.radio0.channel=");
	strcat(str, s_channel);
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("SET_CHANNEL_ERROR:error in uci_lookup_ptr\n");
	}
	if (wlconf->ctx == NULL)
	{
		printf("SET_CHANNEL_ERROR:invalid context\n");
		return -1;
	}
	rv = uci_set(wlconf->ctx, &ptr);
	if (rv != UCI_OK)
	{
		printf("SET_CHANNEL_ERROR:error in uci_set\n");
		return -1;
	}
	uci_save(wlconf->ctx, ptr.p);
	wlconf->conf->channel = channel;
	return 0;

}

static int set_key(struct wlconf *wlconf, char *key)
{
	struct uci_ptr ptr;
	int rv;
	char str[100];
	if (key == NULL)
	{
		strcpy(str, "wireless.@wifi-iface[0].key");
		if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
		{
			printf("SET_KEY_ERROR:error in uci_lookup_ptr\n");
		}
		rv = uci_delete(wlconf->ctx, &ptr);
		if (rv != UCI_OK)
		{
			printf("SET_KEY_ERROR:error in uci_set\n");
			return -1;
		}
		uci_save(wlconf->ctx, ptr.p);
		return 0;
	}
	strcpy(str, "wireless.@wifi-iface[0].key=");
	strcat(str, key);
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("SET_KEY_ERROR:error in uci_lookup_ptr\n");
	}
	if (wlconf->ctx == NULL)
	{
		printf("SET_KEY_ERROR:invalid context\n");
		return -1;
	}
	rv = uci_set(wlconf->ctx, &ptr);
	if (rv != UCI_OK)
	{
		printf("SET_KEY_ERROR:error in uci_set\n");
		return -1;
	}
	uci_save(wlconf->ctx, ptr.p);
	strcpy(wlconf->conf->key, key);
	return 0;
}

static int set_encryption(struct wlconf *wlconf, char *encryption)
{
	struct uci_ptr ptr;
	int rv;
	char str[100];
	if (!(strcmp(encryption, NO_ENCRYPTION)))
	{
		set_key(wlconf, NULL);
	}
	strcpy(str, "wireless.@wifi-iface[0].encryption=");
	strcat(str, encryption);
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("SET_ENCRYPTION_ERROR:error in uci_lookup_ptr\n");
	}
	if (wlconf->ctx == NULL)
	{
		printf("SET_ENCRYPTION_ERROR:invalid context\n");
		return -1;
	}
	rv = uci_set(wlconf->ctx, &ptr);
	if (rv != UCI_OK)
	{
		printf("SET_ENCRYPTION_ERROR:error in uci_set\n");
		return -1;
	}
	uci_save(wlconf->ctx, ptr.p);
	strcpy(wlconf->conf->encryption, encryption);
	return 0;
}

static int set_hwmode(struct wlconf *wlconf, char *hwmode)
{
	struct uci_ptr ptr;
	int rv;
	char str[100];
	strcpy(str, "wireless.radio0.hwmode=");
	strcat(str, hwmode);
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("SET_HWMODE_ERROR:error in uci_lookup_ptr\n");
	}
	if (wlconf->ctx == NULL)
	{
		printf("SET_HWMODE_ERROR:invalid context\n");
		return -1;
	}
	rv = uci_set(wlconf->ctx, &ptr);
	if (rv != UCI_OK)
	{
		printf("SET_HWMODE_ERROR:error in uci_set\n");
		return -1;
	}
	uci_save(wlconf->ctx, ptr.p);
	strcpy(wlconf->conf->hwmode, hwmode);
	return 0;
}

static int set_ssid_hidden(struct wlconf *wlconf, bool hidden)
{
	struct uci_ptr ptr;
	char str[100];
	strcpy(str, "wireless.@wifi-iface[0].hidden");
	int rv;
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("SET_SSID_HIDE_ERROR:error in uci_lookup_ptr\n");
		return -1;
	}
	if (hidden)
	{
		ptr.value = "1";
		rv = uci_set(wlconf->ctx, &ptr);
		if (rv != UCI_OK)
		{
			printf("SET_SSID_HIDE_ERROR:error in uci_set\n");
			return -1;
		}
		uci_save(wlconf->ctx, ptr.p);
		wlconf->conf->hidden = true;
		return 0;
	}
	else
	{
		rv = uci_delete(wlconf->ctx, &ptr);
		if (rv != UCI_OK)
		{
			printf("SET_SSID_HIDE_ERROR:error in uci_delete\n");
			return -1;
		}
		uci_save(wlconf->ctx, ptr.p);
		wlconf->conf->hidden = true;
		return 0;
	}
}

static int set_macfilter(struct wlconf *wlconf, char *mode)
{
	struct maclist_node *node;
	struct uci_ptr ptr;
	char str[100];
	int rv;
	strcpy(str, "wireless.@wifi-iface[0].macfilter");
	if (wlconf->ctx == NULL)
	{
		printf("SET_FILTER_ERROR:invalid context\n");
		return -1;
	}
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("SET_FILTER_ERROR:error in uci_lookup_ptr\n");
		return -1;
	}
	if (!(strcmp(mode, "none")))
	{
		mlist_foreach_element(wlconf->conf->macfilter_list, node)
		{
			del_macfilterlist(wlconf, node->macaddr);
		}
		strcpy(wlconf->conf->macfilter, mode);
		rv = uci_delete(wlconf->ctx, &ptr);
		if (rv != UCI_OK)
		{
			printf("SET_FILTER_ERROR:error in uci_delete\n");
			return -1;
		}
		uci_save(wlconf->ctx, ptr.p);
		strcpy(wlconf->conf->macfilter, mode);
		return 0;
	}
	else 
	{
		ptr.value = mode;
		rv = uci_set(wlconf->ctx, &ptr);
		if (rv != UCI_OK)
		{
			printf("SET_FILTER_ERROR:error in uci_set\n");
			return -1;
		}
		uci_save(wlconf->ctx, ptr.p);
		strcpy(wlconf->conf->macfilter, mode);
		return 0;
	}
}


static int change_commit(struct wlconf *wlconf)
{
	struct uci_ptr ptr;
	char str[] = "wireless";
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("error in uci_lookup_ptr\n");
		return -1;
	}
	if (uci_commit(wlconf->ctx, &ptr.p, false) != UCI_OK) {
		printf("commit error!\n");
		return -1;
	}
}

static int clear_macfilterlist(struct wlconf *wlconf)
{
	struct maclist_node *node;
	mlist_foreach_element(wlconf->conf->macfilter_list, node)
	{
		del_macfilterlist(wlconf, node->macaddr);
	}
}

static int set_txpower(struct wlconf *wlconf, int txpower)
{
	struct uci_ptr ptr;
	int rv;
	char str[100];
	char s_txpower[2];
	sprintf(s_txpower, "%d", txpower);
	strcpy(str, "wireless.radio0.txpower=");
	strcat(str, s_txpower);
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("SET_TXPOWER_ERROR:error in uci_lookup_ptr\n");
	}
	if (wlconf->ctx == NULL)
	{
		printf("SET_TXPOWER_ERROR:invalid context\n");
		return -1;
	}
	rv = uci_set(wlconf->ctx, &ptr);
	if (rv != UCI_OK)
	{
		printf("SET_TXPOWER_ERROR:error in uci_set\n");
		return -1;
	}
	uci_save(wlconf->ctx, ptr.p);
	wlconf->conf->txpower = txpower;
	return 0;
}

static void init_conf(struct wlconf *wlconf)
{
	struct uci_element *section_e;
	uci_foreach_element(&(wlconf->pkg)->sections, section_e)
	{
		struct uci_section *s = uci_to_section(section_e);
		struct uci_element *option_e;
		uci_foreach_element(&(s->options), option_e)
		{
			struct uci_option *o = uci_to_option(option_e);
			char *ele_name = o->e.name;
			char *value = strdup(o->v.string);
			if (!(strcmp(ele_name, "channel")))
			{
				int channel = atoi(value);
				wlconf->conf->channel = channel;
			}
			else if (!(strcmp(ele_name, "txpower")))
			{
				int txpower = atoi(value);
				// printf("txpower:%d", txpower);
				wlconf->conf->txpower = txpower;
			}
			else if (!(strcmp(ele_name, "ssid")))
			{
				strcpy(wlconf->conf->ssid, value);
			}
			else if (!(strcmp(ele_name, "encryption")))
			{
				strcpy(wlconf->conf->encryption, value);
			}
			else if (!(strcmp(ele_name, "key")))
			{
				strcpy(wlconf->conf->key, value);
			}
			else if (!(strcmp(ele_name, "macfilter")))
			{
				strcpy(wlconf->conf->macfilter, value);
			}
			else if (!(strcmp(ele_name, "maclist")))
			{
				struct uci_element *macaddr;
				uci_foreach_element(&(o->v.list), macaddr)
				{
					wlconf->conf->macfilter_list->add_member(wlconf->conf->macfilter_list, macaddr->name);
				}
			}
			else if (!(strcmp(ele_name, "hwmode")))
			{
				strcpy(wlconf->conf->hwmode, value);
			}
			else if (!(strcmp(ele_name, "hidden")))
			{
				if (!(strcmp(value, "1")))
				{
					wlconf->conf->hidden = true;
				}
				else
				{
					wlconf->conf->hidden = false;
				}
			}
			else
			{
				wlconf->conf->hidden = false;
			}
			free (value);
		}
	}
}

static int update(struct wlconf *wlconf)
{
	uci_unload(wlconf->ctx, wlconf->pkg);
	if (uci_load(wlconf->ctx, "wireless", &(wlconf->pkg)) != UCI_OK)
	{
		printf ("wlconf_ALLOC:error in uci_load\n");
		wlconf_free(wlconf);
		return NULL;
	}
	init_conf(wlconf);
}

static void init_func(struct wlconf *wlconf)
{
	wlconf->set_ssid = &set_ssid;
	wlconf->set_channel = &set_channel;
	wlconf->set_encryption = &set_encryption;
	wlconf->set_key = &set_key;
	wlconf->set_macfilter = &set_macfilter;
	wlconf->set_hwmode = &set_hwmode;
	wlconf->set_ssid_hidden = &set_ssid_hidden;
	wlconf->add_macfilterlist = &add_macfilterlist;
	wlconf->del_macfilterlist = &del_macfilterlist;
	wlconf->change_commit = &change_commit;
	wlconf->clear_macfilterlist = &clear_macfilterlist;
	wlconf->set_txpower = &set_txpower;
	wlconf->update = &update;
}

static int init_wlconf(struct wlconf *wlconf)
{
	init_conf(wlconf);
	init_func(wlconf);
}

struct maclist *maclist_alloc()
{
	struct maclist *list = malloc(sizeof(struct maclist));
	list->listsize = 0;
	list->head = NULL;
	list->tail = NULL;
	list->add_member = &add_member;
	list->del_member = &del_member;
	list->free_maclist = &free_maclist;
	return list;
}

int wlconf_free(struct wlconf *wlconf)
{
	/*
	 * commit change after all the saves
	 */
	struct uci_ptr ptr;
	char str[] = "wireless";
	if (uci_lookup_ptr(wlconf->ctx, &ptr, str, true) != UCI_OK)
	{
		printf("error in uci_lookup_ptr\n");
		return -1;
	}
	// if (uci_commit(wlconf->ctx, &ptr.p, false) != UCI_OK) {
	// 	printf("commit error!\n");
	// 	return-1;
	// }

	uci_unload(wlconf->ctx, wlconf->pkg);
	// printf("FREE:unload(uci_package)\n");
	uci_free_context(wlconf->ctx);
	// printf("FREE:uci_context\n");
	if (wlconf->conf->macfilter_list->listsize != 0)
	{
		// printf("FREE:macfilter_list_node\n");
		free_maclist(wlconf->conf->macfilter_list);
	}
	// printf("FREE:macfilter_list\n");
	free(wlconf->conf->macfilter_list);
	// printf("FREE:uci_conf\n");
	free(wlconf->conf);
	free(wlconf);
	return 0;
}

struct wlconf *wlconf_alloc(void)
{
	struct wlconf *wlconf = malloc(sizeof(struct wlconf));
	struct uci_conf *conf = malloc(sizeof(struct uci_conf));
	struct uci_ptr ptr;
	wlconf->conf = conf;
	wlconf->conf->macfilter_list = maclist_alloc();
	wlconf->ctx = uci_alloc_context();
	if (uci_load(wlconf->ctx, "wireless", &(wlconf->pkg)) != UCI_OK)
	{
		printf ("wlconf_ALLOC:error in uci_load\n");
		wlconf_free(wlconf);
		return NULL;
	}
	init_wlconf(wlconf);
	return wlconf;
}
