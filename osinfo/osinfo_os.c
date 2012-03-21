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

G_DEFINE_TYPE (OsinfoOs, osinfo_os, OSINFO_TYPE_PRODUCT);

#define OSINFO_OS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), OSINFO_TYPE_OS, OsinfoOsPrivate))

/**
 * SECTION:osinfo_os
 * @short_description: An operating system
 * @see_also: #OsinfoOs, #OsinfoDeployment
 *
 * #OsinfoOs is an entity representing an operating system.
 * Operating systems have a list of supported devices.
 * There are relationships amongst operating systems to
 * declare which are newest releases, which are clones
 * and which are derived from a common ancestry.
 */

struct _OsinfoOsPrivate
{
    // Value: List of device_link structs
    GList *deviceLinks;

    OsinfoMediaList *medias;
    OsinfoTreeList *trees;
    OsinfoResourcesList *minimum;
    OsinfoResourcesList *recommended;
};

struct _OsinfoOsDeviceLink {
    OsinfoDevice *dev;
    gchar *driver;
};

enum {
    PROP_0,

    PROP_FAMILY,
    PROP_DISTRO,
};

static void osinfo_os_finalize (GObject *object);

static void osinfo_device_link_free(gpointer data, gpointer opaque G_GNUC_UNUSED)
{
    g_object_unref(OSINFO_DEVICELINK(data));
}

static void
osinfo_os_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    OsinfoEntity *entity = OSINFO_ENTITY (object);

    switch (property_id)
        {
        case PROP_FAMILY:
            g_value_set_string (value,
                                osinfo_entity_get_param_value (entity,
                                                               "family"));
            break;
        case PROP_DISTRO:
            g_value_set_string (value,
                                osinfo_entity_get_param_value (entity,
                                                               "distro"));
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
        }
}

static void
osinfo_os_finalize (GObject *object)
{
    OsinfoOs *os = OSINFO_OS (object);

    g_list_foreach(os->priv->deviceLinks, osinfo_device_link_free, NULL);
    g_list_free(os->priv->deviceLinks);
    g_object_unref(os->priv->medias);
    g_object_unref(os->priv->trees);

    /* Chain up to the parent class */
    G_OBJECT_CLASS (osinfo_os_parent_class)->finalize (object);
}

/* Init functions */
static void
osinfo_os_class_init (OsinfoOsClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    g_klass->get_property = osinfo_os_get_property;
    g_klass->finalize = osinfo_os_finalize;

    g_type_class_add_private (klass, sizeof (OsinfoOsPrivate));

    /**
     * OsinfoOs:family:
     *
     * The generic family this OS belongs to, based upon its kernel,
     * for example Linux, Windows, Solaris, FreeBSD etc.
     */
    pspec = g_param_spec_string ("family",
                                 "Family",
                                 "Generic Family",
                                 NULL /* default value */,
                                 G_PARAM_READABLE |
                                 G_PARAM_STATIC_NAME |
                                 G_PARAM_STATIC_NICK |
                                 G_PARAM_STATIC_BLURB);
    g_object_class_install_property (g_klass,
                                     PROP_FAMILY,
                                     pspec);

    /**
     * OsinfoOs:distro:
     *
     * The generic distro this OS belongs to, for example fedora, windows,
     * solaris, freebsd etc.
     */
    pspec = g_param_spec_string ("distro",
                                 "Distro",
                                 "Generic Distro",
                                 NULL /* default value */,
                                 G_PARAM_READABLE |
                                 G_PARAM_STATIC_NAME |
                                 G_PARAM_STATIC_NICK |
                                 G_PARAM_STATIC_BLURB);
    g_object_class_install_property (g_klass,
                                     PROP_DISTRO,
                                     pspec);
}

static void
osinfo_os_init (OsinfoOs *os)
{
    OsinfoOsPrivate *priv;
    os->priv = priv = OSINFO_OS_GET_PRIVATE(os);

    os->priv->deviceLinks = NULL;
    os->priv->medias = osinfo_medialist_new ();
    os->priv->trees = osinfo_treelist_new ();
    os->priv->minimum = osinfo_resourceslist_new ();
    os->priv->recommended = osinfo_resourceslist_new ();
}

/**
 * osinfo_os_new:
 * @id: a unique identifier
 *
 * Create a new operating system entity
 *
 * Returns: (transfer full): a new operating system entity
 */
OsinfoOs *osinfo_os_new(const gchar *id)
{
    return g_object_new(OSINFO_TYPE_OS,
                        "id", id,
                        NULL);
}


/**
 * osinfo_os_get_devices:
 * @os: an operating system
 * @filter: (allow-none)(transfer none): an optional device property filter
 *
 * Get all devices matching a given filter
 *
 * Returns: (transfer full): A list of devices
 */
OsinfoDeviceList *osinfo_os_get_devices(OsinfoOs *os, OsinfoFilter *filter)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(!filter || OSINFO_IS_FILTER(filter), NULL);

    OsinfoDeviceList *newList = osinfo_devicelist_new();
    GList *tmp = NULL;

    tmp = os->priv->deviceLinks;

    while (tmp) {
        OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(tmp->data);
        OsinfoDevice *dev = osinfo_devicelink_get_target(devlink);
        if (!filter || osinfo_filter_matches(filter, OSINFO_ENTITY(dev)))
            osinfo_list_add(OSINFO_LIST(newList), OSINFO_ENTITY(dev));

        tmp = tmp->next;
    }

    return newList;
}

/**
 * osinfo_os_get_all_devices:
 * @os: an operating system
 * @filter: (allow-none)(transfer none): an optional device property filter
 *
 * Get all devices matching a given filter but unlike osinfo_os_get_devices
 * this function also retreives devices from all derived and cloned operating
 * systems.
 *
 * Returns: (transfer full): A list of devices
 */
OsinfoDeviceList *osinfo_os_get_all_devices(OsinfoOs *os, OsinfoFilter *filter)
{
    OsinfoProductList *derived, *cloned, *related_list;
    OsinfoDeviceList *devices;
    guint i;

    devices = osinfo_os_get_devices(os, filter);

    derived = osinfo_product_get_related
                (OSINFO_PRODUCT(os), OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM);
    cloned = osinfo_product_get_related(OSINFO_PRODUCT(os),
                                        OSINFO_PRODUCT_RELATIONSHIP_CLONES);
    related_list = osinfo_productlist_new_union(derived, cloned);
    g_object_unref(derived);
    g_object_unref(cloned);

    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(related_list)); i++) {
        OsinfoEntity *related;
        OsinfoDeviceList *related_devices;

        related = osinfo_list_get_nth(OSINFO_LIST(related_list), i);
        related_devices = osinfo_os_get_all_devices(OSINFO_OS(related), filter);
        if (osinfo_list_get_length(OSINFO_LIST(related_devices)) > 0) {
            OsinfoDeviceList *tmp_list = devices;
            devices = osinfo_devicelist_new_union(devices, related_devices);
            g_object_unref(tmp_list);
        }
    }

    g_object_unref (related_list);

    return devices;
}

/**
 * osinfo_os_get_devices_by_property:
 * @os: an operating system
 * @property: the property of interest
 * @value: the required value of property @property
 * @inherited: Should devices from inherited and cloned OSs be included in the
 * search.
 *
 * A utility function that gets devices found from the list of devices
 * @os supports, for which the value of @property is @value.
 *
 * Returns: (transfer full): The found devices
 */
OsinfoDeviceList *osinfo_os_get_devices_by_property(OsinfoOs *os,
                                                    const gchar *property,
                                                    const gchar *value,
                                                    gboolean inherited)
{
    OsinfoDeviceList *devices;
    OsinfoFilter *filter;

    filter = osinfo_filter_new();
    osinfo_filter_add_constraint(filter, property, value);

    if (inherited)
        devices = osinfo_os_get_all_devices(os, filter);
    else
        devices = osinfo_os_get_devices(os, filter);
    g_object_unref (filter);

    return devices;
}

/**
 * osinfo_os_get_device_links:
 * @os: an operating system
 * @filter: (allow-none)(transfer none): an optional device property filter
 *
 * Get all devices matching a given filter. The filter
 * matches against the links, not the devices.
 *
 * Returns: (transfer full): A list of device links
 */
OsinfoDeviceLinkList *osinfo_os_get_device_links(OsinfoOs *os, OsinfoFilter *filter)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(!filter || OSINFO_IS_FILTER(filter), NULL);

    OsinfoDeviceLinkList *newList = osinfo_devicelinklist_new();
    GList *tmp = NULL;

    tmp = os->priv->deviceLinks;

    while (tmp) {
        OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(tmp->data);

        if (!filter || osinfo_filter_matches(filter, OSINFO_ENTITY(devlink)))
            osinfo_list_add(OSINFO_LIST(newList), OSINFO_ENTITY(devlink));

        tmp = tmp->next;
    }

    return newList;
}


/**
 * osinfo_os_add_device:
 * @os: an operating system
 * @dev: (transfer none): the device to associate with
 *
 * Associated a device with an operating system.  The
 * returned #OsinfoDeviceLink can be used to record
 * extra metadata against the link
 *
 * Returns: (transfer none): the device association
 */
OsinfoDeviceLink *osinfo_os_add_device(OsinfoOs *os, OsinfoDevice *dev)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(OSINFO_IS_DEVICE(dev), NULL);

    OsinfoDeviceLink *devlink = osinfo_devicelink_new(dev);

    os->priv->deviceLinks = g_list_append(os->priv->deviceLinks, devlink);

    return devlink;
}

/**
 * osinfo_os_get_family:
 * @os: a OsinfoOs
 *
 * Retrieves the generic family the OS @os belongs to, based upon its kernel,
 * for example Linux, Windows, Solaris, FreeBSD etc.
 *
 * Returns: (transfer none): the family of this os
 */
const gchar *osinfo_os_get_family(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    return osinfo_entity_get_param_value(OSINFO_ENTITY(os), "family");
}

/**
 * osinfo_os_get_distro:
 * @os: a OsinfoOs
 *
 * Retrieves the generic family the OS @os belongs to, for example fedora,
 * ubuntu, windows, solaris, freebsd etc.
 *
 * Returns: (transfer none): the distro of this os
 */
const gchar *osinfo_os_get_distro(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    return osinfo_entity_get_param_value(OSINFO_ENTITY(os), "distro");
}

/**
 * osinfo_os_get_media_list:
 * @os: an operating system
 *
 * Get all installation medias associated with operating system @os.
 *
 * Returns: (transfer full): A list of medias
 */
OsinfoMediaList *osinfo_os_get_media_list(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    OsinfoMediaList *newList = osinfo_medialist_new();

    osinfo_list_add_all(OSINFO_LIST(newList), OSINFO_LIST(os->priv->medias));

    return newList;
}

/**
 * osinfo_os_add_media:
 * @os: an operating system
 * @media: (transfer none): the media to add
 *
 * Adds installation media @media to operating system @os.
 */
void osinfo_os_add_media(OsinfoOs *os, OsinfoMedia *media)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_MEDIA(media));

    osinfo_list_add(OSINFO_LIST(os->priv->medias), OSINFO_ENTITY(media));
}

/**
 * osinfo_os_get_tree_list:
 * @os: an operating system
 *
 * Get all installation trees associated with operating system @os.
 *
 * Returns: (transfer full): A list of trees
 */
OsinfoTreeList *osinfo_os_get_tree_list(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    OsinfoTreeList *newList = osinfo_treelist_new();

    osinfo_list_add_all(OSINFO_LIST(newList), OSINFO_LIST(os->priv->trees));

    return newList;
}

/**
 * osinfo_os_add_tree:
 * @os: an operating system
 * @tree: (transfer none): the tree to add
 *
 * Adds installation tree @tree to operating system @os.
 */
void osinfo_os_add_tree(OsinfoOs *os, OsinfoTree *tree)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_TREE(tree));

    osinfo_list_add(OSINFO_LIST(os->priv->trees), OSINFO_ENTITY(tree));
}

/**
 * osinfo_os_get_minimum_resources:
 * @os: an operating system
 *
 * Get the list of minimum required resources for the operating system @os.
 *
 * Returns: (transfer full): A list of resources
 */
OsinfoResourcesList *osinfo_os_get_minimum_resources(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    OsinfoResourcesList *newList = osinfo_resourceslist_new();

    osinfo_list_add_all(OSINFO_LIST(newList), OSINFO_LIST(os->priv->minimum));

    return newList;
}

/**
 * osinfo_os_get_recommended_resources:
 * @os: an operating system
 *
 * Get the list of recommended resources for the operating system @os.
 *
 * Returns: (transfer full): A list of resources
 */
OsinfoResourcesList *osinfo_os_get_recommended_resources(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    OsinfoResourcesList *newList = osinfo_resourceslist_new();

    osinfo_list_add_all(OSINFO_LIST(newList),
                        OSINFO_LIST(os->priv->recommended));

    return newList;
}

/**
 * osinfo_os_add_minimum_resources:
 * @os: an operating system
 * @resources: (transfer none): the resources to add
 *
 * Adds @resources to list of minimum resources of operating system @os.
 */
void osinfo_os_add_minimum_resources(OsinfoOs *os, OsinfoResources *resources)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_RESOURCES(resources));

    osinfo_list_add(OSINFO_LIST(os->priv->minimum), OSINFO_ENTITY(resources));
}

/**
 * osinfo_os_add_recommended_resources:
 * @os: an operating system
 * @resources: (transfer none): the resources to add
 *
 * Adds @resources to list of recommended resources of operating system @os.
 */
void osinfo_os_add_recommended_resources(OsinfoOs *os,
                                         OsinfoResources *resources)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_RESOURCES(resources));

    osinfo_list_add(OSINFO_LIST(os->priv->recommended),
                    OSINFO_ENTITY(resources));
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
