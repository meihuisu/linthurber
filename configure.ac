# GNU Autoconf config

AC_INIT(linthurber, 1.0.1)
AC_CONFIG_AUX_DIR([./aux-config])
AM_INIT_AUTOMAKE()
AC_PROG_RANLIB
AC_PROG_MKDIR_P
AC_PROG_LN_S

# Determine MPI/CC compiler
: ${CFLAGS=""}
AC_PROG_CC

# Check for GNU compiler
if test "x$ac_compiler_gnu" = "xno"; then
AC_MSG_ERROR(["GNU C compiler or MPI wrapper based on GNU is required. Please check your programming environment."])
fi

##check optional large data path 
##LINTHURBER_LARGETDATA_DIR=$CVM_LARGETDATA_DIR/model/linthurber
if test x"$CVM_LARGEDATA_DIR" != x; then
   # test directory existence
   LINTHURBER_LARGEDATA_DIR=$CVM_LARGEDATA_DIR/model/linthurber
# In docker container building.. this is nto accessible  yet
#   AC_CHECK_FILE($CVM_LARGEDATA_DIR/model/linthurber/CVM_CM.vo, [], [AC_MSG_ERROR(["LINTHURBER data not found in cvm_largedata_dir"])])
   AM_CONDITIONAL([WITH_LINTHURBER_LARGEDATA_DIR], true)
   AC_SUBST(LINTHURBER_LARGEDATA_DIR)
else
   AM_CONDITIONAL(WITH_LINTHURBER_LARGEDATA_DIR, false)
fi

CFLAGS="$CFLAGS"
LDFLAGS="$LDFLAGS -lm"

AC_CONFIG_FILES([Makefile
                data/Makefile
                src/Makefile
                tests/Makefile])

AC_OUTPUT
