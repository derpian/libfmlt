/*
 *      menu-cache.c
 *
 *      Copyright 2008 PCMan <pcman.tw@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "version.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#include <gio/gio.h>

#include "menu-cache.h"

#ifdef G_ENABLE_DEBUG
#define DEBUG(...)  g_debug(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#if GLIB_CHECK_VERSION(2, 32, 0)
static GRecMutex _cache_lock;
#  define MENU_CACHE_LOCK       g_rec_mutex_lock(&_cache_lock)
#  define MENU_CACHE_UNLOCK     g_rec_mutex_unlock(&_cache_lock)
#else
/* before 2.32 GLib had another entity for statically allocated mutexes */
static GStaticRecMutex _cache_lock = G_STATIC_REC_MUTEX_INIT;
#  define MENU_CACHE_LOCK       g_static_rec_mutex_lock(&_cache_lock)
#  define MENU_CACHE_UNLOCK     g_static_rec_mutex_unlock(&_cache_lock)
#endif

struct _MenuCacheItem
{
    guint n_ref;
    MenuCacheType type;
    char* id;
    char* name;
    char* comment;
    char* icon;
    const char* file_dir;
    char* file_name;
    MenuCacheDir* parent;
};

struct _MenuCacheDir
{
    MenuCacheItem item;
    GSList* children;
};

struct _MenuCacheApp
{
    MenuCacheItem item;
    const char* file_dir;
    char* generic_name;
    char* exec;
    char* working_dir;
    guint32 show_in_flags;
/*    char** categories;    */
    guint32 flags;
};

struct _MenuCache
{
    guint n_ref;
    MenuCacheDir* root_dir;
    char* menu_name;
    char* reg; /* includes md5 sum */
    char* md5; /* link inside of reg */
    char* cache_file;
    char** all_used_files;
    int n_all_used_files;
    char** known_des;
    GSList* notifiers;
    GThread* thr;
    GCancellable* cancellable;
};

static int server_fd = -1;
static GIOChannel* server_ch = NULL;
static guint server_watch = 0;
static GHashTable* hash = NULL;

/* Don't call this API directly. Use menu_cache_lookup instead. */
static MenuCache* menu_cache_new( const char* cache_file );

static gboolean connect_server(GCancellable* cancellable);
static gboolean register_menu_to_server(MenuCache* cache);
static void unregister_menu_from_server( MenuCache* cache );

/* keep them for backward compatibility */
#ifdef G_DISABLE_DEPRECATED
MenuCacheDir* menu_cache_get_root_dir( MenuCache* cache );
MenuCacheDir* menu_cache_item_get_parent( MenuCacheItem* item );
MenuCacheDir* menu_cache_get_dir_from_path( MenuCache* cache, const char* path );
GSList* menu_cache_dir_get_children( MenuCacheDir* dir );
#endif

void menu_cache_init(int flags)
{

}

static MenuCacheItem* read_item(GDataInputStream* f, MenuCache* cache);

/* functions read_dir(), read_app(), and read_item() should be called for
   items that aren't accessible yet, therefore no lock is required */
static void read_dir(GDataInputStream* f, MenuCacheDir* dir, MenuCache* cache)
{
    MenuCacheItem* item;

    /* load child items in the dir */
    while( (item = read_item( f, cache )) )
    {
        /* menu_cache_ref shouldn't be called here for dir.
         * Otherwise, circular reference will happen. */
        item->parent = dir;
        dir->children = g_slist_prepend( dir->children, item );
    }

    dir->children = g_slist_reverse( dir->children );
}

static void read_app(GDataInputStream* f, MenuCacheApp* app, MenuCache* cache)
{
    char *line;
    gsize len;

    /* generic name */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        return;
    if(G_LIKELY(len > 0))
        app->generic_name = line;
    else
        g_free(line);

    /* exec */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        return;
    if(G_LIKELY(len > 0))
        app->exec = line;
    else
        g_free(line);

    /* terminal / startup notify */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        return;
    app->flags = (guint32)atoi(line);
    g_free(line);

    /* ShowIn flags */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        return;
    app->show_in_flags = (guint32)atol(line);
    g_free(line);
}

static MenuCacheItem* read_item(GDataInputStream* f, MenuCache* cache)
{
    MenuCacheItem* item;
    char *line;
    gsize len;
    gint idx;

    /* desktop/menu id */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        return NULL;

    if( G_LIKELY(len >= 1) )
    {
        if( line[0] == '+' ) /* menu dir */
        {
            item = (MenuCacheItem*)g_slice_new0( MenuCacheDir );
            item->n_ref = 1;
            item->type = MENU_CACHE_TYPE_DIR;
        }
        else if( line[0] == '-' ) /* menu item */
        {
            item = (MenuCacheItem*)g_slice_new0( MenuCacheApp );
            item->n_ref = 1;
            if( G_LIKELY( len > 1 ) ) /* application item */
                item->type = MENU_CACHE_TYPE_APP;
            else /* separator */
            {
                item->type = MENU_CACHE_TYPE_SEP;
                return item;
            }
        }
        else
            return NULL;

        item->id = g_strndup( line + 1, len - 1 );
        g_free(line);
    }
    else
    {
        g_free(line);
        return NULL;
    }

    /* name */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        goto _fail;
    if(G_LIKELY(len > 0))
        item->name = line;
    else
        g_free(line);

    /* comment */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        goto _fail;
    if(G_LIKELY(len > 0))
        item->comment = line;
    else
        g_free(line);

    /* icon */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        goto _fail;
    if(G_LIKELY(len > 0))
        item->icon = line;
    else
        g_free(line);

    /* file dir/basename */

    /* file name */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        goto _fail;
    if(G_LIKELY(len > 0))
        item->file_name = line;
    else if( item->type == MENU_CACHE_TYPE_APP )
    {
        /* When file name is the same as desktop_id, which is
         * quite common in desktop files, we use this trick to
         * save memory usage. */
        item->file_name = item->id;
        g_free(line);
    }
    else
        g_free(line);

    /* desktop file dir */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
    {
_fail:
        g_free(item->id);
        g_free(item->name);
        g_free(item->comment);
        g_free(item->icon);
        if(item->file_name && item->file_name != item->id)
            g_free(item->file_name);
        if(item->type == MENU_CACHE_TYPE_DIR)
            g_slice_free(MenuCacheDir, MENU_CACHE_DIR(item));
        else
            g_slice_free(MenuCacheApp, MENU_CACHE_APP(item));
        return NULL;
    }
    idx = atoi( line );
    g_free(line);
    if( G_LIKELY( idx >=0 && idx < cache->n_all_used_files ) )
        item->file_dir = cache->all_used_files[ idx ] + 1;

    if( item->type == MENU_CACHE_TYPE_DIR )
        read_dir( f, MENU_CACHE_DIR(item), cache );
    else if( item->type == MENU_CACHE_TYPE_APP )
        read_app( f, MENU_CACHE_APP(item), cache );

    return item;
}

static gboolean read_all_used_files(GDataInputStream* f, MenuCache* cache)
{
    char *line;
    gsize len;
    int i, n;
    char** dirs;

    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        return FALSE;

    cache->n_all_used_files = n = atoi( line );
    dirs = g_new0( char*, n + 1 );
    g_free(line);

    for( i = 0; i < n; ++i )
    {
        line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
        if(G_UNLIKELY(line == NULL))
        {
            g_strfreev(dirs);
            cache->n_all_used_files = 0;
            return FALSE;
        }

        dirs[i] = line; /* don't include \n */
    }
    cache->all_used_files = dirs;
    return TRUE;
}

static gboolean read_all_known_des(GDataInputStream* f, MenuCache* cache)
{
    char *line;
    gsize len;
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        return FALSE;
    cache->known_des = g_strsplit_set( line, ";\n", 0 );
    g_free(line);
    return TRUE;
}

static MenuCache* menu_cache_new( const char* cache_file )
{
    MenuCache* cache;
    cache = g_slice_new0( MenuCache );
    cache->cache_file = g_strdup( cache_file );
    cache->n_ref = 1;
    return cache;
}

/**
 * menu_cache_ref
 * @cache: a menu cache descriptor
 *
 * Increases reference counter on @cache.
 *
 * Returns: @cache.
 *
 * Since: 0.1.0
 */
MenuCache* menu_cache_ref(MenuCache* cache)
{
    g_atomic_int_inc( &cache->n_ref );
    return cache;
}

/**
 * menu_cache_unref
 * @cache: a menu cache descriptor
 *
 * Descreases reference counter on @cache. When reference count becomes 0
 * then resources associated with @cache will be freed.
 *
 * Since: 0.1.0
 */
void menu_cache_unref(MenuCache* cache)
{
    /* DEBUG("cache_unref: %d", cache->n_ref); */
    /* we need a lock here unfortunately because item in hash isn't protected
       by reference therefore another thread may get access to it right now */
    MENU_CACHE_LOCK;
    if( g_atomic_int_dec_and_test(&cache->n_ref) )
    {
        unregister_menu_from_server( cache );
        /* DEBUG("unregister to server"); */
        g_hash_table_remove( hash, cache->menu_name );
        if( g_hash_table_size(hash) == 0 )
        {
            /* DEBUG("destroy hash"); */
            g_hash_table_destroy(hash);

            /* DEBUG("disconnect from server"); */
            g_source_remove(server_watch);
            g_io_channel_unref(server_ch);
            server_fd = -1;
            server_ch = NULL;
            server_watch = 0;
            hash = NULL;
        }
        MENU_CACHE_UNLOCK;

        if(G_LIKELY(cache->thr))
        {
            g_cancellable_cancel(cache->cancellable);
            g_thread_join(cache->thr);
        }
        g_object_unref(cache->cancellable);
        if( G_LIKELY(cache->root_dir) )
        {
            /* DEBUG("unref root dir"); */
            menu_cache_item_unref( MENU_CACHE_ITEM(cache->root_dir) );
            /* DEBUG("unref root dir finished"); */
        }
        g_free( cache->cache_file );
        g_free( cache->menu_name );
        g_free(cache->reg);
        /* g_free( cache->menu_file_path ); */
        g_strfreev( cache->all_used_files );
        g_strfreev(cache->known_des);
        g_slist_free(cache->notifiers);
        g_slice_free( MenuCache, cache );
    }
    else
        MENU_CACHE_UNLOCK;
}

/**
 * menu_cache_get_root_dir
 * @cache: a menu cache instance
 *
 * Since: 0.1.0
 *
 * Deprecated: 0.3.4: Use menu_cache_dup_root_dir() instead.
 */
/* FIXME: this is thread-unsafe so should be deprecated in favor
   to some other API which will return referenced data! */
MenuCacheDir* menu_cache_get_root_dir( MenuCache* cache )
{
    return cache->root_dir;
}

/**
 * menu_cache_dup_root_dir
 * @cache: a menu cache instance
 *
 * Retrieves root directory for @cache. Returned data should be freed
 * with menu_cache_item_unref() after usage.
 *
 * Returns: (transfer full): root item or %NULL in case of error.
 *
 * Since: 0.3.4
 */
MenuCacheDir* menu_cache_dup_root_dir( MenuCache* cache )
{
    MenuCacheDir* dir;
    MENU_CACHE_LOCK;
    dir = cache->root_dir;
    if(G_LIKELY(dir))
        menu_cache_item_ref(MENU_CACHE_ITEM(dir));
    MENU_CACHE_UNLOCK;
    return dir;
}

/**
 * menu_cache_item_ref
 * @item: a menu cache item
 *
 * Increases reference counter on @item.
 *
 * Returns: @item.
 *
 * Since: 0.1.0
 */
MenuCacheItem* menu_cache_item_ref(MenuCacheItem* item)
{
    g_atomic_int_inc( &item->n_ref );
    /* DEBUG("item_ref %s: %d -> %d", item->id, item->n_ref-1, item->n_ref); */
    return item;
}

typedef struct _CacheReloadNotifier
{
    MenuCacheReloadNotify func;
    gpointer user_data;
}CacheReloadNotifier;

struct _MenuCacheNotifyId
{
    GSList l;
};

/**
 * menu_cache_add_reload_notify
 * @cache: a menu cache instance
 * @func: callback to call when menu cache is reloaded
 * @user_data: user data provided for @func
 *
 * Adds a @func to list of callbacks that are called each time menu cache
 * is loaded.
 *
 * Returns: an ID of added callback.
 *
 * Since: 0.1.0
 */
MenuCacheNotifyId menu_cache_add_reload_notify(MenuCache* cache, MenuCacheReloadNotify func, gpointer user_data)
{
    GSList* l = g_slist_alloc();
    CacheReloadNotifier* n = g_slice_new(CacheReloadNotifier);
    n->func = func;
    n->user_data = user_data;
    l->data = n;
    MENU_CACHE_LOCK;
    cache->notifiers = g_slist_concat( cache->notifiers, l );
    MENU_CACHE_UNLOCK;
    return (MenuCacheNotifyId)l;
}

/**
 * menu_cache_remove_reload_notify
 * @cache: a menu cache instance
 * @notify_id: an ID of callback
 *
 * Removes @notify_id from list of callbacks added for @cache by previous
 * call to menu_cache_add_reload_notify().
 *
 * Since: 0.1.0
 */
void menu_cache_remove_reload_notify(MenuCache* cache, MenuCacheNotifyId notify_id)
{
    MENU_CACHE_LOCK;
    g_slice_free( CacheReloadNotifier, ((GSList*)notify_id)->data );
    cache->notifiers = g_slist_delete_link( cache->notifiers, (GSList*)notify_id );
    MENU_CACHE_UNLOCK;
}

static gboolean reload_notify(gpointer data)
{
    MenuCache* cache = (MenuCache*)data;
    GSList* l;
    MENU_CACHE_LOCK;
    /* we have it referenced and there is no source removal so no check */
    for( l = cache->notifiers; l; l = l->next )
    {
        CacheReloadNotifier* n = (CacheReloadNotifier*)l->data;
        n->func( cache, n->user_data );
    }
    MENU_CACHE_UNLOCK;
    return FALSE;
}

/**
 * menu_cache_reload
 * @cache: a menu cache instance
 *
 * Reloads menu cache from file generated by menu-cached.
 *
 * Returns: %TRUE if reload was successful.
 *
 * Since: 0.1.0
 */
gboolean menu_cache_reload( MenuCache* cache )
{
    char* line;
    gsize len;
    GFile* file;
    GFileInputStream* istr = NULL;
    GDataInputStream* f;

    file = g_file_new_for_path(cache->cache_file);
    if(!file)
        return FALSE;
    istr = g_file_read(file, cache->cancellable, NULL);
    g_object_unref(file);
    if(!istr)
        return FALSE;
    f = g_data_input_stream_new(G_INPUT_STREAM(istr));
    g_object_unref(istr);
    if( ! f )
        return FALSE;

    /* the first line is version number */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_LIKELY(line))
    {
        int ver_maj, ver_min;
        len = sscanf(line, "%d.%d", &ver_maj, &ver_min);
        g_free(line);
        if(len < 2)
            goto _fail;
        if( ver_maj != VER_MAJOR || ver_min != VER_MINOR )
            goto _fail;
    }
    else
        goto _fail;

    /* the second line is menu name */
    line = g_data_input_stream_read_line(f, &len, cache->cancellable, NULL);
    if(G_UNLIKELY(line == NULL))
        goto _fail;
    g_free(line);

    /* FIXME: this may lock other threads for some time */
    MENU_CACHE_LOCK;
    if(cache->notifiers == NULL)
    {
        /* nobody aware of reloads, stupid clients may think root is forever */
        MENU_CACHE_UNLOCK;
        goto _fail;
    }
    g_strfreev( cache->all_used_files );

    /* get all used files */
    if( ! read_all_used_files( f, cache ) )
    {
        cache->all_used_files = NULL;
        MENU_CACHE_UNLOCK;
        goto _fail;
    }

    /* read known DEs */
    g_strfreev( cache->known_des );
    if( ! read_all_known_des( f, cache ) )
    {
        cache->known_des = NULL;
        MENU_CACHE_UNLOCK;
_fail:
        g_object_unref(f);
        return FALSE;
    }

    if(cache->root_dir)
        menu_cache_item_unref( MENU_CACHE_ITEM(cache->root_dir) );

    cache->root_dir = (MenuCacheDir*)read_item( f, cache );
    g_object_unref(f);

    g_idle_add_full(G_PRIORITY_HIGH_IDLE, reload_notify, menu_cache_ref(cache),
                    (GDestroyNotify)menu_cache_unref);
    MENU_CACHE_UNLOCK;

    return TRUE;
}

/**
 * menu_cache_item_unref
 * @item: a menu cache item
 *
 * Decreases reference counter on @item. When reference count becomes 0
 * then resources associated with @item will be freed.
 *
 * Since: 0.1.0
 */
void menu_cache_item_unref(MenuCacheItem* item)
{
    /* DEBUG("item_unref(%s): %d", item->id, item->n_ref); */
    /* We need a lock here unfortunately since another thread may have access
       to it via some child->parent which isn't protected by reference */
    MENU_CACHE_LOCK; /* lock may be recursive here */
    if( g_atomic_int_dec_and_test( &item->n_ref ) )
    {
        /* DEBUG("free item: %s", item->id); */
        g_free( item->id );
        g_free( item->name );
        g_free( item->comment );
        g_free( item->icon );

        if( item->file_name && item->file_name != item->id )
            g_free( item->file_name );

        if( item->parent )
        {
            /* DEBUG("remove %s from parent %s", item->id, MENU_CACHE_ITEM(item->parent)->id); */
            /* remove ourselve from the parent node. */
            item->parent->children = g_slist_remove(item->parent->children, item);
        }

        if( item->type == MENU_CACHE_TYPE_DIR )
        {
            MenuCacheDir* dir = MENU_CACHE_DIR(item);
            GSList* l;
            for(l = dir->children; l; )
            {
                MenuCacheItem* child = MENU_CACHE_ITEM(l->data);
                /* remove ourselve from the children. */
                child->parent = NULL;
                l = l->next;
                menu_cache_item_unref(child);
            }
            g_slist_free( dir->children );
            g_slice_free( MenuCacheDir, dir );
        }
        else
        {
            MenuCacheApp* app = MENU_CACHE_APP(item);
            g_free( app->exec );
            g_slice_free( MenuCacheApp, app );
        }
    }
    MENU_CACHE_UNLOCK;
}

/**
 * menu_cache_item_get_type
 * @item: a menu cache item
 *
 * Checks type of @item.
 *
 * Returns: type of @item.
 *
 * Since: 0.1.0
 */
MenuCacheType menu_cache_item_get_type( MenuCacheItem* item )
{
    return item->type;
}

/**
 * menu_cache_item_get_id
 * @item: a menu cache item
 *
 * Retrieves ID (short name) of @item. Returned data are owned by menu
 * cache and should be not freed by caller.
 *
 * Returns: (transfer none): item ID.
 *
 * Since: 0.1.0
 */
const char* menu_cache_item_get_id( MenuCacheItem* item )
{
    return item->id;
}

/**
 * menu_cache_item_get_name
 * @item: a menu cache item
 *
 * Retrieves display name of @item. Returned data are owned by menu
 * cache and should be not freed by caller.
 *
 * Returns: (transfer none): @item display name or %NULL.
 *
 * Since: 0.1.0
 */
const char* menu_cache_item_get_name( MenuCacheItem* item )
{
    return item->name;
}

/**
 * menu_cache_item_get_comment
 * @item: a menu cache item
 *
 * Retrieves comment of @item. The comment can be used to show tooltip
 * on @item. Returned data are owned by menu cache and should be not
 * freed by caller.
 *
 * Returns: (transfer none): @item comment or %NULL.
 *
 * Since: 0.1.0
 */
const char* menu_cache_item_get_comment( MenuCacheItem* item )
{
    return item->comment;
}

/**
 * menu_cache_item_get_icon
 * @item: a menu cache item
 *
 * Retrieves name of icon of @item. Returned data are owned by menu
 * cache and should be not freed by caller.
 *
 * Returns: (transfer none): @item icon name or %NULL.
 *
 * Since: 0.1.0
 */
const char* menu_cache_item_get_icon( MenuCacheItem* item )
{
    return item->icon;
}

/**
 * menu_cache_item_get_file_basename
 * @item: a menu cache item
 *
 * Retrieves basename of @item. This API can return %NULL if @item is a
 * directory and have no directory desktop entry file. Returned data are
 * owned by menu cache and should be not freed by caller.
 *
 * Returns: (transfer none): @item file basename or %NULL.
 *
 * Since: 0.2.0
 */
const char* menu_cache_item_get_file_basename( MenuCacheItem* item )
{
    return item->file_name;
}

/**
 * menu_cache_item_get_file_dirname
 * @item: a menu cache item
 *
 * Retrieves path to directory where @item desktop enrty file is located.
 * This API can return %NULL if @item is a directory and have no
 * desktop entry file. Returned data are owned by menu cache and should
 * be not freed by caller.
 *
 * Returns: (transfer none): @item file parent directory path or %NULL.
 *
 * Since: 0.2.0
 */
const char* menu_cache_item_get_file_dirname( MenuCacheItem* item )
{
    return item->file_dir;
}

/**
 * menu_cache_item_get_file_path
 * @item: a menu cache item
 *
 * Retrieves path to @item desktop enrty file. This API can return %NULL
 * if @item is a directory and have no desktop entry file. Returned data
 * should be freed with g_free() after usage.
 *
 * Returns: (transfer full): @item file path or %NULL.
 *
 * Since: 0.2.0
 */
char* menu_cache_item_get_file_path( MenuCacheItem* item )
{
    if( ! item->file_name || ! item->file_dir )
        return NULL;
    return g_build_filename( item->file_dir, item->file_name, NULL );
}

/**
 * menu_cache_item_get_parent
 * @item: a menu cache item
 *
 * Since: 0.1.0
 *
 * Deprecated: 0.3.4: Use menu_cache_item_dup_parent() instead.
 */
/* FIXME: this is thread-unsafe so should be deprecated in favor
   to some other API which will return referenced data! */
MenuCacheDir* menu_cache_item_get_parent( MenuCacheItem* item )
{
    return item->parent;
}

/**
 * menu_cache_item_dup_parent
 * @item: a menu item
 *
 * Retrieves parent (directory) for @item. Returned data should be freed
 * with menu_cache_item_unref() after usage.
 *
 * Returns: (transfer full): parent item or %NULL in case of error.
 *
 * Since: 0.3.4
 */
MenuCacheDir* menu_cache_item_dup_parent( MenuCacheItem* item )
{
    MenuCacheDir* dir;
    MENU_CACHE_LOCK;
    dir = item->parent;
    if(G_LIKELY(dir))
        menu_cache_item_ref(MENU_CACHE_ITEM(dir));
    MENU_CACHE_UNLOCK;
    return dir;
}

/**
 * menu_cache_dir_get_children
 * @dir: a menu cache item
 *
 * Retrieves list of items contained in @dir. Returned data are owned by
 * menu cache and should be not freed by caller.
 * This API is thread unsafe and should be never called from outside of
 * default main loop.
 *
 * Returns: (transfer none) (element-type MenuCacheItem): list of items.
 *
 * Since: 0.1.0
 *
 * Deprecated: 0.4.0: Use menu_cache_dir_list_children() instead.
 */
GSList* menu_cache_dir_get_children( MenuCacheDir* dir )
{
    return dir->children;
}

/**
 * menu_cache_dir_list_children
 * @dir: a menu cache item
 *
 * Retrieves list of items contained in @dir. Returned data should be
 * freed with g_slist_free_full(list, menu_cache_item_unref) after usage.
 *
 * Returns: (transfer full) (element-type MenuCacheItem): list of items.
 *
 * Since: 0.4.0
 */
GSList* menu_cache_dir_list_children(MenuCacheDir* dir)
{
    GSList *children, *l;

    g_return_val_if_fail(MENU_CACHE_ITEM(dir)->type == MENU_CACHE_TYPE_DIR, NULL);
    MENU_CACHE_LOCK;
    children = g_slist_copy(dir->children);
    for(l = children; l; l = l->next)
        menu_cache_item_ref(l->data);
    MENU_CACHE_UNLOCK;
    return children;
}

/**
 * menu_cache_app_get_exec
 * @app: a menu cache item
 *
 * Retrieves execution string for @app. Returned data are owned by menu
 * cache and should be not freed by caller.
 *
 * Returns: (transfer none): item execution string or %NULL.
 *
 * Since: 0.1.0
 */
const char* menu_cache_app_get_exec( MenuCacheApp* app )
{
    return app->exec;
}

/**
 * menu_cache_app_get_working_dir
 * @app: a menu cache item
 *
 * Retrieves working directory for @app. Returned data are owned by menu
 * cache and should be not freed by caller.
 *
 * Returns: (transfer none): item working directory or %NULL.
 *
 * Since: 0.1.0
 */
const char* menu_cache_app_get_working_dir( MenuCacheApp* app )
{
    return app->working_dir;
}

/*
char** menu_cache_app_get_categories( MenuCacheApp* app )
{
    return NULL;
}
*/

/**
 * menu_cache_app_get_use_terminal
 * @app: a menu cache item
 *
 * Checks if @app should be ran in terminal.
 *
 * Returns: %TRUE if @app requires terminal to run.
 *
 * Since: 0.1.0
 */
gboolean menu_cache_app_get_use_terminal( MenuCacheApp* app )
{
    return ( (app->flags & FLAG_USE_TERMINAL) != 0 );
}

/**
 * menu_cache_app_get_use_sn
 * @app: a menu cache item
 *
 * Checks if @app wants startup notification.
 *
 * Returns: %TRUE if @app wants startup notification.
 *
 * Since: 0.1.0
 */
gboolean menu_cache_app_get_use_sn( MenuCacheApp* app )
{
    return ( (app->flags & FLAG_USE_SN) != 0 );
}

/**
 * menu_cache_app_get_show_flags
 * @app: a menu cache item
 *
 * Retrieves list of desktop environments where @app should be visible.
 *
 * Returns: bit mask of DE.
 *
 * Since: 0.2.0
 */
guint32 menu_cache_app_get_show_flags( MenuCacheApp* app )
{
    return app->show_in_flags;
}

/**
 * menu_cache_app_get_is_visible
 * @app: a menu cache item
 * @de_flags: bit mask of DE to test
 *
 * Checks if @app should be visible in any of desktop environments
 * @de_flags.
 *
 * Returns: %TRUE if @app is visible.
 *
 * Since: 0.2.0
 */
gboolean menu_cache_app_get_is_visible( MenuCacheApp* app, guint32 de_flags )
{
    return !app->show_in_flags || (app->show_in_flags & de_flags);
}

/*
MenuCacheApp* menu_cache_find_app_by_exec( const char* exec )
{
    return NULL;
}
*/

/**
 * menu_cache_get_dir_from_path
 * @cache: a menu cache instance
 * @path: item path
 *
 * Since: 0.1.0
 *
 * Deprecated: 0.3.4: Use menu_cache_item_from_path() instead.
 */
MenuCacheDir* menu_cache_get_dir_from_path( MenuCache* cache, const char* path )
{
    char** names = g_strsplit( path + 1, "/", -1 );
    int i = 0;
    MenuCacheDir* dir = NULL;

    if( !names )
        return NULL;

    if( G_UNLIKELY(!names[0]) )
    {
        g_strfreev(names);
        return NULL;
    }
    /* the topmost dir of the path should be the root menu dir. */
    MENU_CACHE_LOCK;
    if( strcmp(names[0], MENU_CACHE_ITEM(cache->root_dir)->id) )
    {
        MENU_CACHE_UNLOCK;
        return NULL;
    }

    dir = cache->root_dir;
    for( ++i; names[i]; ++i )
    {
        GSList* l;
        for( l = dir->children; l; l = l->next )
        {
            MenuCacheItem* item = MENU_CACHE_ITEM(l->data);
            if( item->type == MENU_CACHE_TYPE_DIR && 0 == strcmp( item->id, names[i] ) )
                dir = MENU_CACHE_DIR(item);
        }
        /* FIXME: we really should ref it on return since other thread may
           destroy the parent at this time and returned data become invalid.
           Therefore this call isn't thread-safe! */
        if( ! dir )
        {
            MENU_CACHE_UNLOCK;
            return NULL;
        }
    }
    MENU_CACHE_UNLOCK;
    return dir;
}

/**
 * menu_cache_item_from_path
 * @cache: cache to inspect
 * @path: item path
 *
 * Searches item @path in the @cache. The @path consists of item IDs
 * separated by slash ('/'). Returned data should be freed with
 * menu_cache_item_unref() after usage.
 *
 * Returns: (transfer full): found item or %NULL if no item found.
 *
 * Since: 0.3.4
 */
MenuCacheItem* menu_cache_item_from_path( MenuCache* cache, const char* path )
{
    char** names = g_strsplit( path + 1, "/", -1 );
    int i;
    MenuCacheDir* dir;
    MenuCacheItem* item = NULL;

    if( !names )
        return NULL;

    if( G_UNLIKELY(!names[0]) )
    {
        g_strfreev(names);
        return NULL;
    }
    /* the topmost dir of the path should be the root menu dir. */
    MENU_CACHE_LOCK;
    dir = cache->root_dir;
    if( G_UNLIKELY(!dir) || strcmp(names[0], MENU_CACHE_ITEM(dir)->id) != 0 )
        goto _end;

    for( i = 1; names[i]; ++i )
    {
        GSList* l;
        item = NULL;
        if( !dir )
            break;
        l = dir->children;
        dir = NULL;
        for( ; l; l = l->next )
        {
            item = MENU_CACHE_ITEM(l->data);
            if( strcmp( item->id, names[i] ) == 0 )
            {
                if( item->type == MENU_CACHE_TYPE_DIR )
                    dir = MENU_CACHE_DIR(item);
                break;
            }
        }
        if( !item )
            break;
    }
    if(item)
        menu_cache_item_ref(item);
_end:
    MENU_CACHE_UNLOCK;
    g_strfreev(names);
    return item;
}

/**
 * menu_cache_dir_make_path
 * @dir: a menu cache item
 *
 * Retrieves path of @dir. The path consists of item IDs separated by
 * slash ('/'). Returned data should be freed with g_free() after usage.
 *
 * Returns: (transfer full): item path.
 *
 * Since: 0.1.0
 */
char* menu_cache_dir_make_path( MenuCacheDir* dir )
{
    GString* path = g_string_sized_new(1024);
    MenuCacheItem* it;

    MENU_CACHE_LOCK;
    while( (it = MENU_CACHE_ITEM(dir)) ) /* this is not top dir */
    {
        g_string_prepend( path, menu_cache_item_get_id(it) );
        g_string_prepend_c( path, '/' );
        /* FIXME: if parent is already unref'd by another thread then
           path being made will be broken. Is there any way to avoid that? */
        dir = it->parent;
    }
    MENU_CACHE_UNLOCK;
    return g_string_free( path, FALSE );
}

static void get_socket_name( char* buf, int len )
{
    char* dpy = g_strdup(g_getenv("DISPLAY"));
    if(dpy && *dpy)
    {
        char* p = strchr(dpy, ':');
        for(++p; *p && *p != '.';)
            ++p;
        if(*p)
            *p = '\0';
    }
    g_snprintf( buf, len, "%s/.menu-cached-%s-%s", g_get_tmp_dir(),
                dpy ? dpy : ":0", g_get_user_name() );
    g_free(dpy);
}

#define MAX_RETRIES 25

static gboolean fork_server()
{
    return FALSE;
}

static gboolean on_server_io(GIOChannel* ch, GIOCondition cond, gpointer user_data)
{
    GIOStatus st;
    char* line;
    gsize len;

    if( cond & (G_IO_ERR|G_IO_HUP) )
    {
reconnect:
        DEBUG("IO error %d, try to re-connect.", cond);
        g_io_channel_unref(ch);
        server_fd = -1;
        if( ! connect_server(NULL) )
        {
            g_print("fail to re-connect to the server.\n");
        }
        else
        {
            GHashTableIter it;
            char* menu_name;
            MenuCache* cache;
            DEBUG("successfully restart server.\nre-register menus.");
            /* re-register all menu caches */

            MENU_CACHE_LOCK;
            if(hash)
            {
                g_hash_table_iter_init(&it, hash);
                while(g_hash_table_iter_next(&it, (gpointer*)&menu_name, (gpointer*)&cache))
                    register_menu_to_server(cache);
                    /* FIXME: need we remove it from hash if failed? */
            }
            MENU_CACHE_UNLOCK;
        }
        return FALSE;
    }

    if( cond & (G_IO_IN|G_IO_PRI) )
    {
    retry:
        st = g_io_channel_read_line( ch, &line, &len, NULL, NULL );
        if( st == G_IO_STATUS_AGAIN )
            goto retry;
        if ( st != G_IO_STATUS_NORMAL || len < 4 )
        {
            DEBUG("server IO error!!");
            goto reconnect;
            return FALSE;
        }
        DEBUG("server line: %s", line);
        if( 0 == memcmp( line, "REL:", 4 ) ) /* reload */
        {
            GHashTableIter it;
            char* menu_name;
            MenuCache* cache;
            line[len - 1] = '\0';
            char* menu_cache_id = line + 4;
            DEBUG("server ask us to reload cache: %s", menu_cache_id);

            MENU_CACHE_LOCK;
            g_hash_table_iter_init(&it, hash);
            while(g_hash_table_iter_next(&it, (gpointer*)&menu_name, (gpointer*)&cache))
            {
                if(0 == memcmp(cache->md5, menu_cache_id, 32))
                {
                    DEBUG("RELOAD!");
                    menu_cache_reload(cache);
                    break;
                }
            }
            MENU_CACHE_UNLOCK;
        }
        g_free( line );
    }
    return TRUE;
}

static gboolean update_watch_on_idle(gpointer unused)
{
    MENU_CACHE_LOCK;
    /* always do server interaction in default main loop */
    if(!server_watch)
        server_watch = g_io_add_watch(server_ch, G_IO_IN|G_IO_PRI|G_IO_ERR|G_IO_HUP, on_server_io, NULL);
    MENU_CACHE_UNLOCK;
    return FALSE;
}

G_LOCK_DEFINE(connect);

static gboolean connect_server(GCancellable* cancellable)
{
    int fd, rc;
    struct sockaddr_un addr;
    int retries = 0;

    G_LOCK(connect);
    if(server_fd != -1 || (cancellable && g_cancellable_is_cancelled(cancellable)))
    {
        G_UNLOCK(connect);
        return TRUE;
    }

retry:
    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        g_print("Failed to create socket\n");
        G_UNLOCK(connect);
        return FALSE;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    get_socket_name( addr.sun_path, sizeof( addr.sun_path ) );

    if( connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        rc = errno;
        close(fd);
        if(cancellable && g_cancellable_is_cancelled(cancellable))
        {
            G_UNLOCK(connect);
            return TRUE;
        }
        if((rc == ECONNREFUSED || rc == ENOENT) && retries == 0)
        {
            DEBUG("no running server found, starting it");
            fork_server();
            ++retries;
            goto retry;
        }
        if(retries < MAX_RETRIES)
        {
            usleep(50000);
            ++retries;
            goto retry;
        }
        g_print("Unable to connect\n");
        G_UNLOCK(connect);
        return FALSE;
    }
    server_fd = fd;
    server_ch = g_io_channel_unix_new(fd);
    G_UNLOCK(connect);
    g_io_channel_set_close_on_unref(server_ch, TRUE);
    g_idle_add(update_watch_on_idle, NULL);
    return TRUE;
}

static MenuCache* menu_cache_create(const char* menu_name)
{
    MenuCache* cache;
    const gchar * const * langs = g_get_language_names();
    const char* xdg_cfg = g_getenv("XDG_CONFIG_DIRS");
    const char* xdg_prefix = g_getenv("XDG_MENU_PREFIX");
    const char* xdg_data = g_getenv("XDG_DATA_DIRS");
    const char* xdg_cfg_home = g_getenv("XDG_CONFIG_HOME");
    const char* xdg_data_home = g_getenv("XDG_DATA_HOME");
    const char* xdg_cache_home = g_getenv("XDG_CACHE_HOME");
    char* buf;
    const char* md5;
    char* file_name;
    int len = 0;
    GChecksum *sum;

    if( !xdg_cfg )
        xdg_cfg = "";
    if( ! xdg_prefix )
        xdg_prefix = "";
    if( ! xdg_data )
        xdg_data = "";
    if( ! xdg_cfg_home )
        xdg_cfg_home = "";
    if( ! xdg_data_home )
        xdg_data_home = "";
    if( ! xdg_cache_home )
        xdg_cache_home = "";

    /* get rid of the encoding part of locale name. */
    while( strchr(langs[0], '.') )
        ++langs;

    buf = g_strdup_printf( "REG:%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t00000000000000000000000000000000\n",
                            menu_name,
                            *langs,
                            xdg_cache_home,
                            xdg_cfg,
                            xdg_prefix,
                            xdg_data,
                            xdg_cfg_home,
                            xdg_data_home );

    /* calculate the md5 sum of menu name + lang + all environment variables */
    sum = g_checksum_new(G_CHECKSUM_MD5);
    len = strlen(buf);
    g_checksum_update(sum, (guchar*)buf + 4, len - 38);
    md5 = g_checksum_get_string(sum);
    file_name = g_build_filename( g_get_user_cache_dir(), "menus", md5, NULL );
    DEBUG("cache file_name = %s", file_name);
    cache = menu_cache_new( file_name );
    cache->reg = buf;
    cache->md5 = buf + len - 33;
    memcpy( cache->md5, md5, 32 );
    cache->menu_name = g_strdup(menu_name);
    g_free( file_name );

    g_checksum_free(sum); /* md5 is also freed here */

    MENU_CACHE_LOCK;
    g_hash_table_insert( hash, g_strdup(menu_name), cache );
    MENU_CACHE_UNLOCK;

    return cache;
}

static gboolean register_menu_to_server(MenuCache* cache)
{
    ssize_t len = strlen(cache->reg);
    /* FIXME: do unblocking I/O */
    if(write(server_fd, cache->reg, len) < len)
    {
        DEBUG("register_menu_to_server: sending failed");
        return FALSE; /* socket write failed */
    }
    return TRUE;
}

static void unregister_menu_from_server( MenuCache* cache )
{
    char buf[38];
    g_snprintf( buf, 38, "UNR:%s\n", cache->md5 );
    /* FIXME: do unblocking I/O */
    if(write( server_fd, buf, 37 ) <= 0)
    {
        DEBUG("unregister_menu_from_server: sending failed");
    }
}

static gpointer menu_cache_loader_thread(gpointer data)
{
    MenuCache* cache = (MenuCache*)data;

    /* try to connect server now */
    if(!connect_server(cache->cancellable))
    {
        g_print("unable to connect to menu-cached.\n");
        return NULL;
    }
    /* and request update from server */
    if(!g_cancellable_is_cancelled(cache->cancellable))
        register_menu_to_server(cache);
    return NULL;
}

/**
 * menu_cache_lookup
 * @menu_name: a menu cache root
 *
 * Searches for connection to menu-cached for @menu_name. If no such
 * connection exists then creates new one. Caller can be notified when
 * cache is ready by adding callback.
 *
 * See also: menu_cache_add_reload_notify().
 *
 * Returns: (transfer full): menu cache descriptor.
 *
 * Since: 0.1.0
 */
MenuCache* menu_cache_lookup( const char* menu_name )
{
    MenuCache* cache;

    /* lookup in a hash table for already loaded menus */
    MENU_CACHE_LOCK;
    if( G_UNLIKELY( ! hash ) )
        hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL );
    else
    {
        cache = (MenuCache*)g_hash_table_lookup(hash, menu_name);
        if( cache )
        {
            menu_cache_ref(cache);
            MENU_CACHE_UNLOCK;
            return cache;
        }
    }
    MENU_CACHE_UNLOCK;

    cache = menu_cache_create(menu_name);
    cache->cancellable = g_cancellable_new();
    /* reload existing file first so it will be ready right away */
    menu_cache_reload(cache);
#if GLIB_CHECK_VERSION(2, 32, 0)
    cache->thr = g_thread_new(menu_name, menu_cache_loader_thread, cache);
#else
    cache->thr = g_thread_create(menu_cache_loader_thread, cache, TRUE, NULL);
#endif
    return cache;
}

static void on_menu_cache_reload(MenuCache* mc, gpointer user_data)
{
    GMainLoop* mainloop = (GMainLoop*)user_data;
    g_main_loop_quit(mainloop);
}

/**
 * menu_cache_lookup_sync
 * @menu_name: a menu cache root
 *
 * Searches for data from menu-cached for @menu_name. If no connection
 * exists yet then creates new one and retrieves all data.
 *
 * Returns: (transfer full): menu cache descriptor.
 *
 * Since: 0.3.1
 */
MenuCache* menu_cache_lookup_sync( const char* menu_name )
{
    MenuCache* mc = menu_cache_lookup(menu_name);
    MenuCacheDir* root_dir = menu_cache_dup_root_dir(mc);
    /* ensure that the menu cache is loaded */
    if(root_dir)
        menu_cache_item_unref(MENU_CACHE_ITEM(root_dir));
    else /* if it's not yet loaded */
    {
        /* FIXME: is using mainloop the efficient way to do it? */
        GMainLoop* mainloop = g_main_loop_new(NULL, FALSE);
        MenuCacheNotifyId notify_id;
        notify_id = menu_cache_add_reload_notify(mc, on_menu_cache_reload,
                                                 mainloop);
        g_main_loop_run(mainloop);
        g_main_loop_unref(mainloop);
        menu_cache_remove_reload_notify(mc, notify_id);
    }
    return mc;
}

static GSList* list_app_in_dir(MenuCacheDir* dir, GSList* list)
{
    GSList* l;
    for( l = dir->children; l; l = l->next )
    {
        MenuCacheItem* item = MENU_CACHE_ITEM(l->data);
        switch( menu_cache_item_get_type(item) )
        {
        case MENU_CACHE_TYPE_DIR:
            list = list_app_in_dir( MENU_CACHE_DIR(item), list );
            break;
        case MENU_CACHE_TYPE_APP:
            list = g_slist_prepend(list, menu_cache_item_ref(item));
            break;
        case MENU_CACHE_TYPE_NONE:
        case MENU_CACHE_TYPE_SEP:
            break;
        }
    }
    return list;
}

/**
 * menu_cache_list_all_apps
 * @cache: a menu cache descriptor
 *
 * Retrieves full list of applications in menu cache. Returned list
 * should be freed with g_slist_free_full(list, menu_cache_item_unref)
 * after usage.
 *
 * Returns: (transfer full) (element-type MenuCacheItem): list of items.
 *
 * Since: 0.1.2
 */
GSList* menu_cache_list_all_apps(MenuCache* cache)
{
    GSList* list;
    MENU_CACHE_LOCK;
    list = list_app_in_dir(cache->root_dir, NULL);
    MENU_CACHE_UNLOCK;
    return list;
}

/**
 * menu_cache_get_desktop_env_flag
 * @cache: a menu cache descriptor
 * @desktop_env: desktop environment name
 *
 * Makes bit mask of desktop environment from its name.
 *
 * Returns: DE bit mask.
 *
 * Since: 0.2.0
 */
guint32 menu_cache_get_desktop_env_flag( MenuCache* cache, const char* desktop_env )
{
    char** de;
    MENU_CACHE_LOCK;
    de = cache->known_des;
    if( de )
    {
        int i;
        for( i = 0; de[i]; ++i )
        {
            if( strcmp( desktop_env, de[i] ) == 0 )
            {
                MENU_CACHE_UNLOCK;
                return 1 << (i + N_KNOWN_DESKTOPS);
            }
        }
    }
    MENU_CACHE_UNLOCK;
    if( strcmp(desktop_env, "GNOME") == 0 )
        return SHOW_IN_GNOME;
    if( strcmp(desktop_env, "KDE") == 0 )
        return SHOW_IN_KDE;
    if( strcmp(desktop_env, "XFCE") == 0 )
        return SHOW_IN_XFCE;
    if( strcmp(desktop_env, "LXDE") == 0 )
        return SHOW_IN_LXDE;
    if( strcmp(desktop_env, "ROX") == 0 )
        return SHOW_IN_ROX;
    return 0;
}
