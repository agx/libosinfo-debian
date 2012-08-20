/*
 * libosinfo:
 *
 * Copyright (C) 2009-2012 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors:
 *   Arjun Roy <arroy@redhat.com>
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>
#include <gio/gio.h>
#include <string.h>

G_DEFINE_TYPE (OsinfoDb, osinfo_db, G_TYPE_OBJECT);

#define OSINFO_DB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), OSINFO_TYPE_DB, OsinfoDbPrivate))

#define match_regex(pattern, str)                                       \
    (((pattern) == NULL) ||                                             \
     (((str) != NULL) &&                                                \
      g_regex_match_simple((pattern), (str), 0, 0)))

/**
 * SECTION:osinfo_db
 * @short_description: Database of all entities
 * @see_also: #OsinfoList, #OsinfoEntity
 *
 * #OsinfoDb is a database tracking all entity instances against which
 * metadata is recorded.
 */

struct _OsinfoDbPrivate
{
    OsinfoDeviceList *devices;
    OsinfoPlatformList *platforms;
    OsinfoOsList *oses;
    OsinfoDeploymentList *deployments;
    OsinfoInstallScriptList *scripts;
};

static void osinfo_db_finalize (GObject *object);

static void
osinfo_db_finalize (GObject *object)
{
    OsinfoDb *db = OSINFO_DB (object);

    g_object_unref(db->priv->devices);
    g_object_unref(db->priv->platforms);
    g_object_unref(db->priv->oses);
    g_object_unref(db->priv->deployments);
    g_object_unref(db->priv->scripts);

    /* Chain up to the parent class */
    G_OBJECT_CLASS (osinfo_db_parent_class)->finalize (object);
}


/* Init functions */
static void
osinfo_db_class_init (OsinfoDbClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS (klass);

    g_klass->finalize = osinfo_db_finalize;

    g_type_class_add_private (klass, sizeof (OsinfoDbPrivate));
}


static void
osinfo_db_init (OsinfoDb *db)
{
    OsinfoDbPrivate *priv;
    db->priv = priv = OSINFO_DB_GET_PRIVATE(db);

    db->priv->devices = osinfo_devicelist_new();
    db->priv->platforms = osinfo_platformlist_new();
    db->priv->oses = osinfo_oslist_new();
    db->priv->deployments = osinfo_deploymentlist_new();
    db->priv->scripts = osinfo_install_scriptlist_new();
}

/** PUBLIC METHODS */


/**
 * osinfo_db_new:
 *
 * Returns: (transfer full): the new database
 */
OsinfoDb *osinfo_db_new(void)
{
    return g_object_new(OSINFO_TYPE_DB, NULL);
}

/**
 * osinfo_db_get_platform:
 * @db: the database
 * @id: the unique platform identifier
 *
 * Returns: (transfer none): the platform, or NULL if none is found
 */
OsinfoPlatform *osinfo_db_get_platform(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_PLATFORM(osinfo_list_find_by_id(OSINFO_LIST(db->priv->platforms), id));
}

/**
 * osinfo_db_get_device:
 * @db: the database
 * @id: the unique device identifier
 *
 * Returns: (transfer none): the device, or NULL if none is found
 */
OsinfoDevice *osinfo_db_get_device(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_DEVICE(osinfo_list_find_by_id(OSINFO_LIST(db->priv->devices), id));
}

/**
 * osinfo_db_get_os:
 * @db: the database
 * @id: the unique operating system identifier
 *
 * Returns: (transfer none): the operating system, or NULL if none is found
 */
OsinfoOs *osinfo_db_get_os(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_OS(osinfo_list_find_by_id(OSINFO_LIST(db->priv->oses), id));
}

/**
 * osinfo_db_get_deployment:
 * @db: the database
 * @id: the unique operating system identifier
 *
 * Returns: (transfer none): the operating system, or NULL if none is found
 */
OsinfoDeployment *osinfo_db_get_deployment(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_DEPLOYMENT(osinfo_list_find_by_id(OSINFO_LIST(db->priv->deployments), id));
}

/**
 * osinfo_db_get_install_script:
 * @db: the database
 * @id: the unique operating system identifier
 *
 * Returns: (transfer none): the install script, or NULL if none is found
 */
OsinfoInstallScript *osinfo_db_get_install_script(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_INSTALL_SCRIPT(osinfo_list_find_by_id(OSINFO_LIST(db->priv->scripts), id));
}


/**
 * osinfo_db_find_deployment:
 * @db: the database
 * @os: the operating system to find
 * @platform: the virtualization platform
 *
 * Find the deployment for @os on @platform, if any.
 *
 * Returns: (transfer none): the deployment, or NULL
 */
OsinfoDeployment *osinfo_db_find_deployment(OsinfoDb *db,
                                            OsinfoOs *os,
                                            OsinfoPlatform *platform)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(OSINFO_IS_PLATFORM(platform), NULL);

    GList *deployments = osinfo_list_get_elements(OSINFO_LIST(db->priv->deployments));
    GList *tmp = deployments;

    while (tmp) {
        OsinfoDeployment *deployment = OSINFO_DEPLOYMENT(tmp->data);
        OsinfoOs *otheros = osinfo_deployment_get_os(deployment);
        OsinfoPlatform *otherplatform = osinfo_deployment_get_platform(deployment);

        if (g_strcmp0(osinfo_entity_get_id(OSINFO_ENTITY(os)),
                      osinfo_entity_get_id(OSINFO_ENTITY(otheros))) == 0 &&
            g_strcmp0(osinfo_entity_get_id(OSINFO_ENTITY(platform)),
                      osinfo_entity_get_id(OSINFO_ENTITY(otherplatform))) == 0) {
            g_list_free(deployments);
            return deployment;
        }

        tmp = tmp->next;
    }

    g_list_free(deployments);
    return NULL;
}


/**
 * osinfo_db_get_os_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of operating systems
 */
OsinfoOsList *osinfo_db_get_os_list(OsinfoDb *db)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);

    return osinfo_oslist_new_copy(db->priv->oses);
}

/**
 * osinfo_db_get_platform_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of platforms
 */
OsinfoPlatformList *osinfo_db_get_platform_list(OsinfoDb *db)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);

    return osinfo_platformlist_new_copy(db->priv->platforms);
}

/**
 * osinfo_db_get_device_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of devices
 */
OsinfoDeviceList *osinfo_db_get_device_list(OsinfoDb *db)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);

    return osinfo_devicelist_new_copy(db->priv->devices);
}


/**
 * osinfo_db_get_deployment_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of deployments
 */
OsinfoDeploymentList *osinfo_db_get_deployment_list(OsinfoDb *db)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);

    return osinfo_deploymentlist_new_copy(db->priv->deployments);
}


/**
 * osinfo_db_get_install_script_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of install scripts
 */
OsinfoInstallScriptList *osinfo_db_get_install_script_list(OsinfoDb *db)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);

    return osinfo_install_scriptlist_new_copy(db->priv->scripts);
}


/**
 * osinfo_db_add_os:
 * @db: the database
 * @os: (transfer none): an operating system
 *
 */
void osinfo_db_add_os(OsinfoDb *db, OsinfoOs *os)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_OS(os));

    osinfo_list_add(OSINFO_LIST(db->priv->oses), OSINFO_ENTITY(os));
}


/**
 * osinfo_db_add_platform:
 * @db: the database
 * @platform: (transfer none): an platform
 *
 */
void osinfo_db_add_platform(OsinfoDb *db, OsinfoPlatform *platform)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_PLATFORM(platform));

    osinfo_list_add(OSINFO_LIST(db->priv->platforms), OSINFO_ENTITY(platform));
}


/**
 * osinfo_db_add_device:
 * @db: the database
 * @device: (transfer none): a device
 *
 */
void osinfo_db_add_device(OsinfoDb *db, OsinfoDevice *device)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_DEVICE(device));

    osinfo_list_add(OSINFO_LIST(db->priv->devices), OSINFO_ENTITY(device));
}


/**
 * osinfo_db_add_deployment:
 * @db: the database
 * @deployment: (transfer none): a deployment
 *
 */
void osinfo_db_add_deployment(OsinfoDb *db, OsinfoDeployment *deployment)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_DEPLOYMENT(deployment));

    osinfo_list_add(OSINFO_LIST(db->priv->deployments), OSINFO_ENTITY(deployment));
}


/**
 * osinfo_db_add_install_script:
 * @db: the database
 * @script: (transfer none): a install script
 *
 */
void osinfo_db_add_install_script(OsinfoDb *db, OsinfoInstallScript *script)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_INSTALL_SCRIPT(script));

    osinfo_list_add(OSINFO_LIST(db->priv->scripts), OSINFO_ENTITY(script));
}


static gint media_volume_compare (gconstpointer a, gconstpointer b)
{
    OsinfoMedia *media_a = OSINFO_MEDIA(a);
    OsinfoMedia *media_b = OSINFO_MEDIA(b);
    const gchar *volume_a = osinfo_media_get_volume_id(media_a);
    const gchar *volume_b = osinfo_media_get_volume_id(media_b);

    if (volume_a == NULL || volume_b == NULL)
        /* Order doesn't matter then */
        return 0;

    if (strstr(volume_a, volume_b) != NULL)
        return -1;
    else
        /* Sub-string comes later */
        return 1;
}

/**
 * osinfo_db_guess_os_from_media:
 * @db: the database
 * @media: the installation media
 * @matched_media: (out) (transfer none) (allow-none): the matched operating
 * system media
 *
 * Guess operating system given a #OsinfoMedia object.
 *
 * Returns: (transfer none): the operating system, or NULL if guessing failed
 */
OsinfoOs *osinfo_db_guess_os_from_media(OsinfoDb *db,
                                        OsinfoMedia *media,
                                        OsinfoMedia **matched_media)
{
    OsinfoOs *ret = NULL;
    GList *oss = NULL;
    GList *os_iter;
    const gchar *media_volume;
    const gchar *media_system;
    const gchar *media_publisher;
    const gchar *media_application;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(media != NULL, NULL);

    media_volume = osinfo_media_get_volume_id(media);
    media_system = osinfo_media_get_system_id(media);
    media_publisher = osinfo_media_get_publisher_id(media);
    media_application = osinfo_media_get_application_id(media);

    oss = osinfo_list_get_elements(OSINFO_LIST(db->priv->oses));
    for (os_iter = oss; os_iter; os_iter = os_iter->next) {
        OsinfoOs *os = OSINFO_OS(os_iter->data);
        OsinfoMediaList *media_list = osinfo_os_get_media_list(os);
        GList *medias = osinfo_list_get_elements(OSINFO_LIST(media_list));
        GList *media_iter;

        medias = g_list_sort(medias, media_volume_compare);

        for (media_iter = medias; media_iter; media_iter = media_iter->next) {
            OsinfoMedia *os_media = OSINFO_MEDIA(media_iter->data);
            const gchar *os_volume = osinfo_media_get_volume_id(os_media);
            const gchar *os_system = osinfo_media_get_system_id(os_media);
            const gchar *os_publisher = osinfo_media_get_publisher_id(os_media);
            const gchar *os_application = osinfo_media_get_application_id(os_media);

            if (match_regex (os_volume, media_volume) &&
                match_regex (os_application, media_application) &&
                match_regex (os_system, media_system) &&
                match_regex (os_publisher, media_publisher)) {
                ret = os;
                if (matched_media != NULL)
                    *matched_media = os_media;
                break;
            }
        }

        g_list_free(medias);
        g_object_unref(media_list);

        if (ret)
            break;
    }

    g_list_free(oss);

    return ret;
}


/**
 * osinfo_db_guess_os_from_tree:
 * @db: the database
 * @tree: the installation tree
 * @matched_tree: (out) (transfer none) (allow-none): the matched operating
 * system tree
 *
 * Guess operating system given a #OsinfoTree object.
 *
 * Returns: (transfer none): the operating system, or NULL if guessing failed
 */
OsinfoOs *osinfo_db_guess_os_from_tree(OsinfoDb *db,
                                       OsinfoTree *tree,
                                       OsinfoTree **matched_tree)
{
    OsinfoOs *ret = NULL;
    GList *oss = NULL;
    GList *os_iter;
    const gchar *tree_family;
    const gchar *tree_variant;
    const gchar *tree_version;
    const gchar *tree_arch;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(tree != NULL, NULL);

    tree_family = osinfo_tree_get_treeinfo_family(tree);
    tree_variant = osinfo_tree_get_treeinfo_variant(tree);
    tree_version = osinfo_tree_get_treeinfo_version(tree);
    tree_arch = osinfo_tree_get_treeinfo_arch(tree);

    oss = osinfo_list_get_elements(OSINFO_LIST(db->priv->oses));
    for (os_iter = oss; os_iter; os_iter = os_iter->next) {
        OsinfoOs *os = OSINFO_OS(os_iter->data);
        OsinfoTreeList *tree_list = osinfo_os_get_tree_list(os);
        GList *trees = osinfo_list_get_elements(OSINFO_LIST(tree_list));
        GList *tree_iter;

        //trees = g_list_sort(trees, tree_family_compare);

        for (tree_iter = trees; tree_iter; tree_iter = tree_iter->next) {
            OsinfoTree *os_tree = OSINFO_TREE(tree_iter->data);
            const gchar *os_family = osinfo_tree_get_treeinfo_family(os_tree);
            const gchar *os_variant = osinfo_tree_get_treeinfo_variant(os_tree);
            const gchar *os_version = osinfo_tree_get_treeinfo_version(os_tree);
            const gchar *os_arch = osinfo_tree_get_treeinfo_arch(os_tree);

            if (match_regex (os_family, tree_family) &&
                match_regex (os_variant, tree_variant) &&
                match_regex (os_version, tree_version) &&
                match_regex (os_arch, tree_arch)) {
                ret = os;
                if (matched_tree != NULL)
                    *matched_tree = os_tree;
                break;
            }
        }

        g_list_free(trees);
        g_object_unref(tree_list);

        if (ret)
            break;
    }

    g_list_free(oss);

    return ret;
}

struct osinfo_db_populate_values_args {
    GHashTable *values;
    const gchar *property;
};

static void osinfo_db_get_property_values_in_entity(gpointer data, gpointer opaque)
{
    OsinfoEntity *entity = OSINFO_ENTITY(data);
    struct osinfo_db_populate_values_args *args = opaque;
    GHashTable *newValues = args->values;
    const gchar *property = args->property;
    GList *values = osinfo_entity_get_param_value_list(entity, property);
    GList *tmp = values;

    while (tmp) {
        gchar *value = tmp->data;

        if (!g_hash_table_lookup(newValues, value)) {
            g_hash_table_insert(newValues,
                                value,
                                GINT_TO_POINTER(1));
        }

        tmp = tmp->next;
    }

    g_list_free(values);
}

static GList *osinfo_db_unique_values_for_property_in_entity(OsinfoList *entities, const gchar *propName)
{
    /* Delibrately no free func for key, since we return those to caller */
    GHashTable *values = g_hash_table_new(g_str_hash, g_str_equal);
    GList *ret;
    struct osinfo_db_populate_values_args args = { values, propName };
    GList *elements;

    elements = osinfo_list_get_elements(entities);
    g_list_foreach(elements, osinfo_db_get_property_values_in_entity, &args);
    g_list_free(elements);

    ret = g_hash_table_get_keys(values);
    g_hash_table_unref(values);
    return ret;
}

/**
 * osinfo_db_unique_values_for_property_in_os:
 * @db: the database
 * @propName: a property name
 *
 * Get all unique values for a named property amongst all
 * operating systems in the database
 *
 * Returns: (transfer container)(element-type utf8): a list of strings
 */
GList *osinfo_db_unique_values_for_property_in_os(OsinfoDb *db, const gchar *propName)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(propName != NULL, NULL);

    return osinfo_db_unique_values_for_property_in_entity(OSINFO_LIST(db->priv->oses), propName);
}


/**
 * osinfo_db_unique_values_for_property_in_platform:
 * @db: the database
 * @propName: a property name
 *
 * Get all unique values for a named property amongst all
 * platforms in the database
 *
 * Returns: (transfer container)(element-type utf8): a list of strings
 */
GList *osinfo_db_unique_values_for_property_in_platform(OsinfoDb *db, const gchar *propName)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(propName != NULL, NULL);

    return osinfo_db_unique_values_for_property_in_entity(OSINFO_LIST(db->priv->platforms), propName);
}


/**
 * osinfo_db_unique_values_for_property_in_device:
 * @db: the database
 * @propName: a property name
 *
 * Get all unique values for a named property amongst all
 * devices in the database
 *
 * Returns: (transfer container)(element-type utf8): a list of strings
 */
GList *osinfo_db_unique_values_for_property_in_device(OsinfoDb *db, const gchar *propName)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(propName != NULL, NULL);

    return osinfo_db_unique_values_for_property_in_entity(OSINFO_LIST(db->priv->devices), propName);
}

/**
 * osinfo_db_unique_values_for_property_in_deployment:
 * @db: the database
 * @propName: a property name
 *
 * Get all unique values for a named property amongst all
 * deployments in the database
 *
 * Returns: (transfer container)(element-type utf8): a list of strings
 */
GList *osinfo_db_unique_values_for_property_in_deployment(OsinfoDb *db, const gchar *propName)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(propName != NULL, NULL);

    return osinfo_db_unique_values_for_property_in_entity(OSINFO_LIST(db->priv->deployments), propName);
}

struct __osinfoProductCheckRelationshipArgs {
    OsinfoList *list;
    OsinfoProductRelationship relshp;
};


static void __osinfoAddProductIfRelationship(gpointer data, gpointer opaque)
{
    struct __osinfoProductCheckRelationshipArgs *args = opaque;
    OsinfoProduct *product = OSINFO_PRODUCT(data);
    OsinfoList *newList = args->list;
    OsinfoProductList *thisList = osinfo_product_get_related(product, args->relshp);
    int i;

    for (i = 0 ; i < osinfo_list_get_length(OSINFO_LIST(thisList)) ; i++) {
        OsinfoEntity *entity = osinfo_list_get_nth(OSINFO_LIST(thisList), i);
        osinfo_list_add(newList, entity);
    }

    g_object_unref(thisList);
}

/**
 * osinfo_db_unique_values_for_os_relationship:
 * @db: the database
 * @relshp: the product relationship
 *
 * Get all operating systems that are the referee
 * in an operating system relationship.
 *
 * Returns: (transfer full): a list of operating systems
 */
OsinfoOsList *osinfo_db_unique_values_for_os_relationship(OsinfoDb *db, OsinfoProductRelationship relshp)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);

    OsinfoOsList *newList = osinfo_oslist_new();
    struct __osinfoProductCheckRelationshipArgs args = {OSINFO_LIST (newList), relshp};
    GList *entities = osinfo_list_get_elements(OSINFO_LIST(db->priv->oses));

    g_list_foreach(entities, __osinfoAddProductIfRelationship, &args);
    g_list_free(entities);

    return newList;
}


/**
 * osinfo_db_unique_values_for_platform_relationship:
 * @db: the database
 * @relshp: the product relationship
 *
 * Get all platforms that are the referee
 * in an platform relationship.
 *
 * Returns: (transfer full): a list of virtualization platforms
 */
OsinfoPlatformList *osinfo_db_unique_values_for_platform_relationship(OsinfoDb *db, OsinfoProductRelationship relshp)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);

    OsinfoPlatformList *newList = osinfo_platformlist_new();
    struct __osinfoProductCheckRelationshipArgs args = {OSINFO_LIST (newList), relshp};
    GList *entities = osinfo_list_get_elements(OSINFO_LIST(db->priv->platforms));

    g_list_foreach(entities, __osinfoAddProductIfRelationship, &args);
    g_list_free(entities);

    return newList;
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
