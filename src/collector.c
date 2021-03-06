#include "collector.h"
#include "pg_utils.h"

PG_PUBLIC_API pg_m_item_t pg_start_collecting(char *path, pg_mtype_t type) {
    if (!path) return NULL; /* no path */
    if (type <= PG_MEASUREMENT_TYPE_UNKNOWN) return NULL; /* no type */

    /* this is the new measurement object that will collect all data */
    pg_m_item_t measurement = pg_create_measurement_item();
    /* if no path allocate the new one */
    if (!measurement->path) {
        measurement->path = PG_STRDUP(path);
    }

    /* set the type */
    measurement->type = type;

    return measurement;
}

/* ========================================================================= */

PG_PUBLIC_API pg_err_t pg_stop_collecting(pg_m_item_t measurement) {
    if (!measurement)
        return PG_ERR_NO_MEASUREMENT; // no measurement to destroy

    /* publish the results */
    int result = pg_publish_measurement(measurement);

    /* we destroy the measurement anyway */
    result = pg_destroy_measurement_item(measurement);

    return result;
}

/* ========================================================================= */

PG_PUBLIC_API pg_err_t pg_publish_measurement(pg_m_item_t measurement) {
    if (!measurement)
        return PG_ERR_NO_MEASUREMENT; // no measurement to destroy

    /* create an item for the queue */
    pg_q_item_t item = pg_create_queue_item();
    item->measurement = pg_create_measurement_item();

    /* copy the passed measurement */
    int result = pg_copy_measurement_item(measurement, item->measurement);

    /* enqueue the item */
    result = pg_enqueue(item);

    /* after enqueue we can delete the measurement */
    if (result == 0) {
        result = pg_destroy_queue_item(item);
    }

    return result;
}

/* ========================================================================= */

PG_PUBLIC_API pg_err_t pg_increase_hit(pg_m_item_t measurement) {
    if (!measurement)
        return PG_ERR_NO_MEASUREMENT; // no measurement to update
    if (measurement->type != PG_MEASUREMENT_TYPE_HIT)
        return PG_ERR_WRONG_MEASUREMENT_TYPE; // wrong type

    measurement->hitValue++;
    return PG_NO_ERROR;
}