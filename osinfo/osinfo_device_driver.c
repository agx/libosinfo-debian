/*
 * libosinfo: Device driver
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
 *   Zeeshan Ali <zeenix@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>

#include "osinfo_device_driver_private.h"

G_DEFINE_TYPE (OsinfoDeviceDriver, osinfo_device_driver, OSINFO_TYPE_ENTITY);

#define OSINFO_DEVICE_DRIVER_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                      OSINFO_TYPE_DEVICE_DRIVER, \
                                      OsinfoDeviceDriverPrivate))

/**
 * SECTION:osinfo_device_driver
 * @short_description: Information about device driver
 * @see_also: #OsinfoOs
 *
 * #OsinfoDeviceDriver is an entity representing device drivers for an (guest)
 * operating system.
 */

struct _OsinfoDeviceDriverPrivate
{
    OsinfoDeviceList *devices;
};

static void
osinfo_device_driver_finalize (GObject *object)
{
    OsinfoDeviceDriver *driver = OSINFO_DEVICE_DRIVER (object);

    g_object_unref(driver->priv->devices);

    /* Chain up to the parent class */
    G_OBJECT_CLASS (osinfo_device_driver_parent_class)->finalize (object);
}

/* Init functions */
static void
osinfo_device_driver_class_init (OsinfoDeviceDriverClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS (klass);

    g_klass->finalize = osinfo_device_driver_finalize;
    g_type_class_add_private (klass, sizeof (OsinfoDeviceDriverPrivate));
}

static void
osinfo_device_driver_init (OsinfoDeviceDriver *driver)
{
    OsinfoDeviceDriverPrivate *priv;
    driver->priv = priv = OSINFO_DEVICE_DRIVER_GET_PRIVATE(driver);

    priv->devices = osinfo_devicelist_new ();
}

OsinfoDeviceDriver *osinfo_device_driver_new(const gchar *id)
{
    OsinfoDeviceDriver *driver;

    driver = g_object_new (OSINFO_TYPE_DEVICE_DRIVER,
                           "id", id,
                           NULL);

    return driver;
}

/**
 * osinfo_device_driver_get_architecture:
 * @driver: a #OsinfoDeviceDriver instance
 *
 * Retrieves the target hardware architecture of @driver.
 *
 * Returns: (transfer none): the hardware architecture.
 */
const gchar *osinfo_device_driver_get_architecture(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(driver),
                                         OSINFO_DEVICE_DRIVER_PROP_ARCHITECTURE);
}

/**
 * osinfo_device_driver_get_location:
 * @driver: a #OsinfoDeviceDriver instance
 *
 * Retrieves the location of the @driver as a URL.
 *
 * Returns: (transfer none): the location of the driver.
 */
const gchar *osinfo_device_driver_get_location(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(driver),
                                         OSINFO_DEVICE_DRIVER_PROP_LOCATION);
}

/**
 * osinfo_device_driver_get_files:
 * @driver: a #OsinfoDeviceDriver instance
 *
 * Retrieves the names of driver files under the location returned by
 * #osinfo_device_driver_get_location.
 *
 * Returns: (transfer container) (element-type utf8): The list of driver files.
 */
GList *osinfo_device_driver_get_files(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value_list(OSINFO_ENTITY(driver),
                                              OSINFO_DEVICE_DRIVER_PROP_FILE);
}

/**
 * osinfo_device_driver_get_pre_installable:
 * @driver: a #OsinfoDeviceDriver instance
 *
 * Returns: TRUE if @driver is pre-installable, FALSE otherwise.
 */
gboolean osinfo_device_driver_get_pre_installable(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value_boolean
                (OSINFO_ENTITY(driver),
                 OSINFO_DEVICE_DRIVER_PROP_PRE_INSTALLABLE);
}

/**
 * osinfo_device_driver_get_devices:
 * @driver: a #OsinfoDeviceDriver instance
 *
 * Returns: (transfer none): The list of devices supported by this driver.
 */
OsinfoDeviceList *osinfo_device_driver_get_devices(OsinfoDeviceDriver *driver)
{
    g_return_val_if_fail(OSINFO_IS_DEVICE_DRIVER(driver), NULL);

    return driver->priv->devices;
}

void osinfo_device_driver_add_device(OsinfoDeviceDriver *driver,
                                     OsinfoDevice *device)
{
    g_return_if_fail(OSINFO_IS_DEVICE_DRIVER(driver));
    g_return_if_fail(OSINFO_IS_DEVICE(device));

    osinfo_list_add(OSINFO_LIST(driver->priv->devices),
                    OSINFO_ENTITY(device));
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */