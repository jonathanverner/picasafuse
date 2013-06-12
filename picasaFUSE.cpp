// main.cpp
#include "config.h"
#include "picasaFS.h"
#include <fuse/fuse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>



enum {
  KEY_HELP,
  KEY_VERSION,
};

const char PICASA_FUSE_VERSION[] = "v0.5";

#define MYFS_OPT(t, p, v) { t, offsetof(struct myfs_config, p), v }

static struct fuse_opt myfs_opts[] = {
  MYFS_OPT("update-interval=%i",	updateInterval, 0),
  MYFS_OPT("-t %i",             	updateInterval, 0),
  MYFS_OPT("cachedir=%s",       	cacheDir, 0),
  MYFS_OPT("username=%s",       	userName, 0),
  MYFS_OPT("passsword=%s",		password, 0),
  MYFS_OPT("resize=%i",			maxPixels, 0),
  MYFS_OPT("--offline",			offline, 1 ),
#ifdef HAVE_DBUS
  MYFS_OPT("--use-keyring=false",       useKeyRing, 0 ),
#endif

  FUSE_OPT_KEY("-V",             KEY_VERSION),
  FUSE_OPT_KEY("--version",      KEY_VERSION),
  FUSE_OPT_KEY("-h",             KEY_HELP),
  FUSE_OPT_KEY("--help",         KEY_HELP),
  NULL
  //FUSE_OPT_END
};

static int myfs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs)
{
  if ( key == KEY_HELP || key == KEY_VERSION ) {
    PicasaFS picasa;
    switch (key) {
      case KEY_HELP:
	fprintf(stderr,
	       "usage: %s mountpoint [options]\n"
	       "\n"
	       "general options:\n"
	       "    -o opt,[opt...]  mount options\n"
	       "    -h   --help      print help\n"
	       "    -V   --version   print version\n"
	       "\n"
	       "PicasaFUSE options:\n"
	       "    -o update-interval=NUM	update interval in seconds\n"
	       "    -o cachedir=STRING		directory to store cached data\n"
	       "    -o username=STRING		picasa username to authenticate as\n"
	       "    -o password=STRING		the password for username (CAUTION PASSING PASSWORD ON THE COMMANDLINE IS INSECURE)\n"
	       "    -o resize=NUM		resize to at most NUM pixels when uploading pictures\n"
	       "    --offline			do not try any network operations, work locally\n"
#ifdef HAVE_DBUS
	       "    --use-keyring=false		do not try to use the kde wallet\n"
#endif
	       "    -t NUM           		same as '-o update-interval=NUM'\n\n"
	       , outargs->argv[0]);
	fuse_opt_add_arg(outargs, "-ho");
	picasa.main( outargs->argc, outargs->argv, NULL, &picasa );
	exit(1);
      case KEY_VERSION:
	fprintf(stderr, "PicasaFUSE version %s\n", PICASA_FUSE_VERSION );
	fuse_opt_add_arg(outargs, "--version");
	picasa.main( outargs->argc, outargs->argv, NULL, &picasa );
	exit(0);
    }
  }
  return 1;
}


int main (int argc, char **argv) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  struct myfs_config conf;

  memset(&conf, 0, sizeof(conf));
  conf.offline=false;
#ifdef HAVE_DBUS
  conf.useKeyRing = true;
#endif

  fuse_opt_parse(&args, &conf, myfs_opts, myfs_opt_proc);
  picasaConfig pcConfig(conf);

  PicasaFS picasa( pcConfig );

  // The first 3 parameters are identical to the fuse_main function.
  // The last parameter gives a pointer to a class instance, which is
  // required for static methods to access instance variables/ methods.
  return picasa.main (args.argc, args.argv, NULL, &picasa);
}

