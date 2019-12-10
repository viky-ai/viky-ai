# Matomo

To enable pages tracking using [Matomo](https://matomo.org/), you must define the following environment variables:

* `VIKYAPP_MATOMO_ENABLED`: boolean.
* `VIKYAPP_MATOMO_TRACKER_URL`: Matomo tracker URL.
* `VIKYAPP_MATOMO_SITE_ID`: Matomo site ID.

For example:

    # Enabled Matomo pages tracking
    VIKYAPP_MATOMO_ENABLED=true
    VIKYAPP_MATOMO_TRACKER_URL=https://yourvikyai.matomo.cloud/
    VIKYAPP_MATOMO_SITE_ID=1
