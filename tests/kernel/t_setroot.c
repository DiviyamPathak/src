#include <atf-c.h>
#include <sys/device_if.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/conf.h>
#include <stdio.h>
#include <string.h>

#define	NODEV	(dev_t)(-1)
#define DV_DISK 1
#define DV_IFNET 2


typedef struct device { int dummy; } *device_t;

const char *rootspec = "wd0";
const char *rootdvm;
const char *bootdvm = "sd0";
dev_t rootdev;

void setroot_root(device_t,int);

int devsw_name2blk(const char *name, void *v, int i);
const char *devsw_blk2name(int major);
dev_t MAKEDISKDEV(int maj, int unit, int part);
dev_t makedev(int maj, int unit);
int major(dev_t *dev);
int device_class(device_t dev);
int device_unit(device_t dev);
int DEV_USES_PARTITIONS(device_t dev);
int DISKUNIT(dev_t dev);
int DISKPART(dev_t dev);
device_t parsedisk(const char *spec, int len, int flag, dev_t *dev);
device_t finddevice(const char *name);
const char *device_xname(device_t dev);
void aprint_normal(const char *fmt, ...);
void setroot_dump(device_t dev, void *v);



int 
devsw_name2blk(const char *name, void *v, int i)
{ return 0; }
const char *
devsw_blk2name(int major)
 { return "wd"; }
dev_t 
MAKEDISKDEV(int maj, int unit, int part) 
{ return (dev_t)((maj << 8) | (unit << 4) | part); }
dev_t
makedev(int maj, int unit) { return (dev_t)((maj << 8) | unit); }

int 
major(dev_t dev) { return (dev >> 8) & 0xff; }

int 
device_class(device_t dev) { return DV_DISK; }

int 
device_unit(device_t dev) { return 0; }

int 
DEV_USES_PARTITIONS(device_t dev) { return 1; }

int 
DISKUNIT(dev_t dev) { return (dev >> 4) & 0xf; }

int 
DISKPART(dev_t dev) { return dev & 0xf; }

device_t 
parsedisk(const char *spec, int len, int flag, dev_t *dev) { return NULL; }

device_t 
finddevice(const char *name) { return NULL; }

const char *
device_xname(device_t dev) { return "wd0"; }

void 
aprint_normal(const char *fmt, ...) { printf(fmt); }

void 
setroot_dump(device_t dev, void *v) { }

device_t root_device;

void 
setroot_root(device_t bootdv, int bootpartition) {
    device_t rootdv;
    int majdev;
    const char *rootdevname;
    char buf[128];
    dev_t nrootdev;

        if (rootspec == NULL) {

            /*
            * Wildcarded root; use the boot device.
            */
            rootdv = bootdv;

            if (bootdv)
                majdev = devsw_name2blk(device_xname(bootdv), NULL, 0);
            else
                majdev = -1;
            if (majdev >= 0) {
                /*
                * Root is on a disk.  `bootpartition' is root,
                * unless the device does not use partitions.
                */
                if (DEV_USES_PARTITIONS(bootdv))
                    rootdev = MAKEDISKDEV(majdev,
                                device_unit(bootdv),
                                bootpartition);
                else
                    rootdev = makedev(majdev, device_unit(bootdv));
            }
        } else {

            /*
            * `root on <dev> ...'
            */

            /*
            * If rootspec can be parsed, just use it.
            */
            rootdv = parsedisk(rootspec, strlen(rootspec), 0, &nrootdev);
            if (rootdv != NULL) {
                rootdev = nrootdev;
                goto haveroot;
            }

            /*
            * Fall back to rootdev, compute rootdv for it
            */
            rootdevname = devsw_blk2name(major(rootdev));
            if (rootdevname == NULL) {
                printf("unknown device major 0x%llx\n",
                    (unsigned long long)rootdev);
                return;
            }
            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "%s%llu", rootdevname,
                (unsigned long long)DISKUNIT(rootdev));

            rootdv = finddevice(buf);
            if (rootdv == NULL) {
                printf("device %s (0x%llx) not configured\n",
                    buf, (unsigned long long)rootdev);
                return;
            }
        }

    haveroot:
        switch (device_class(rootdv)) {
        case DV_IFNET:
        case DV_DISK:
            aprint_normal("root on %s", device_xname(rootdv));
            if (DEV_USES_PARTITIONS(rootdv))
                aprint_normal("%c", (int)DISKPART(rootdev) + 'a');
            break;
        default:
            printf("can't determine root device\n");
            return;
        }

        root_device = rootdv;
        setroot_dump(rootdv, NULL);
}

ATF_TC(testsetroot);
ATF_TC_HEAD(testsetroot, tc) {
    atf_tc_set_md_var(tc, "descr", "test setroot");
}

ATF_TC_BODY(testsetroot, tc) {
    setroot_root();
    ATF_CHECK_STREQ("wd0", rootdvm);
}

ATF_TC(testsetroot2);
ATF_TC_HEAD(testsetroot2, tc) {
    atf_tc_set_md_var(tc, "descr", "impl");
}

ATF_TC_BODY(testsetroot2, tc) {
}

ATF_TP_ADD_TCS(tp) {
    ATF_TP_ADD_TC(tp, testsetroot);
    ATF_TP_ADD_TC(tp, testsetroot2);
    return atf_no_error();
}
