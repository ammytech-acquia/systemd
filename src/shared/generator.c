/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

/***
  This file is part of systemd.

  Copyright 2014 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <string.h>
#include <unistd.h>

#include "util.h"
#include "special.h"
#include "mkdir.h"
#include "unit-name.h"
#include "generator.h"
#include "path-util.h"
#include "fstab-util.h"
#include "dropin.h"

int generator_write_fsck_deps(
                FILE *f,
                const char *dest,
                const char *what,
                const char *where,
                const char *fstype) {

        assert(f);
        assert(dest);
        assert(what);
        assert(where);

        if (!is_device_path(what)) {
                log_warning("Checking was requested for \"%s\", but it is not a device.", what);
                return 0;
        }

        if (!isempty(fstype) && !streq(fstype, "auto")) {
                int r;
                r = fsck_exists(fstype);
                if (r == -ENOENT) {
                        /* treat missing check as essentially OK */
                        log_debug_errno(r, "Checking was requested for %s, but fsck.%s does not exist: %m", what, fstype);
                        return 0;
                } else if (r < 0)
                        return log_warning_errno(r, "Checking was requested for %s, but fsck.%s cannot be used: %m", what, fstype);
        }

        if (streq(where, "/")) {
                char *lnk;

                lnk = strjoina(dest, "/" SPECIAL_LOCAL_FS_TARGET ".wants/systemd-fsck-root.service");

                mkdir_parents(lnk, 0755);
                if (symlink(SYSTEM_DATA_UNIT_PATH "/systemd-fsck-root.service", lnk) < 0)
                        return log_error_errno(errno, "Failed to create symlink %s: %m", lnk);

        } else {
                _cleanup_free_ char *fsck = NULL;

                fsck = unit_name_from_path_instance("systemd-fsck", what, ".service");
                if (!fsck)
                        return log_oom();

                fprintf(f,
                        "RequiresOverridable=%s\n"
                        "After=%s\n",
                        fsck,
                        fsck);
        }

        return 0;
}

int generator_write_timeouts(const char *dir, const char *what, const char *where,
                             const char *opts, char **filtered) {

        /* Allow configuration how long we wait for a device that
         * backs a mount point to show up. This is useful to support
         * endless device timeouts for devices that show up only after
         * user input, like crypto devices. */

        _cleanup_free_ char *node = NULL, *unit = NULL, *timeout = NULL;
        usec_t u;
        int r;

        r = fstab_filter_options(opts, "comment=systemd.device-timeout\0" "x-systemd.device-timeout\0",
                                 NULL, &timeout, filtered);
        if (r <= 0)
                return r;

        r = parse_sec(timeout, &u);
        if (r < 0) {
                log_warning("Failed to parse timeout for %s, ignoring: %s",
                            where, timeout);
                return 0;
        }

        node = fstab_node_to_udev_node(what);
        if (!node)
                return log_oom();

        unit = unit_name_from_path(node, ".device");
        if (!unit)
                return log_oom();

        return write_drop_in_format(dir, unit, 50, "device-timeout",
                                    "# Automatically generated by %s\n\n"
                                    "[Unit]\nJobTimeoutSec=" USEC_FMT,
                                    program_invocation_short_name,
                                    u / USEC_PER_SEC);
}
