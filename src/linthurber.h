/**
 * @file linthurber.h
 * @brief Main header file for LINTHURBER cvm library.
 * @author - SCEC
 * @version 1.0.1
 *
 * Delivers LINTHURBER Community Velocity Model
 *
 */

#include "ucvm_dtypes.h"

// Constants
/* Property constants */
#define LINTHURBER_DEM 0
#define LINTHURBER_VP 1
#define LINTHURBER_VS 2

/* Flags */
#define LINTHURBER_USE_DEM 1

#define LINTHURBER_MAX_Z_DIM 100

// Structures
/** Defines a point (latitude, longitude, and depth) in WGS84 format */
typedef struct linthurber_point_t {
	/** Longitude member of the point */
	double longitude;
	/** Latitude member of the point */
	double latitude;
	/** Depth member of the point */
	double depth;
} linthurber_point_t;

/** Defines the material properties this model will retrieve. */
typedef struct linthurber_properties_t {
	/** P-wave velocity in meters per second */
	double vp;
	/** S-wave velocity in meters per second */
	double vs;
	/** Density in g/m^3 */
	double rho;
        /** NOT USED from basic_property_t */
        double qp;
        /** NOT USED from basic_property_t */
        double qs;
} linthurber_properties_t;

/** The LINTHURBER configuration structure. */
typedef struct linthurber_configuration_t {
	/** The zone of UTM projection */
	int utm_zone;
	/** The model directory */
	char model_dir[128];

        double spacing_vp;
        double spacing_vs;
        double spacing_dem;

	int num_z = 0;

	// projection bounding box
        double proj_xi[4];
        double proj_yi[4];
        double proj_dims[2];

        double depths_msl[LINTHURBER_MAX_Z_DIM];
        double grid_origin[2];

	// calculated
        int vp_dims[3];
        int vs_dims[3];
        int dem_dims[3];

        /** Bilinear or Trilinear Interpolation on or off (1 or 0) */
        int interpolation;

} linthurber_configuration_t;

/** The model structure which points to available portions of the model. */
typedef struct linthurber_model_t {
	/** A pointer to the Vp data either in memory or disk. Null if does not exist. */
	void *vp;
	/** A pointer to the Vs data either in memory or disk. Null if does not exist. */
	void *vs;
	/** A pointer to the dem data either in memory or disk. Null if does not exist. */
	void *dem;
	/** status: 0 = not found, 1 = found and not in memory, 2 = found and in memory */
	int vp_status;
	int vs_status;
	int dem_status;
} linthurber_model_t;

// UCVM API Required Functions
#ifdef DYNAMIC_LIBRARY
/** Initializes the model */
int model_init(const char *dir, const char *label);
/** Cleans up the model (frees memory, etc.) */
int model_finalize();
/** Returns version information */
int model_version(char *ver, int len);
/** Queries the model */
int model_query(linthurber_point_t *points, linthurber_properties_t *data, int numpts);
#endif
