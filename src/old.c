/*
 * @file linthurber.c
 * @brief
 * @author - SCEC
 * @version 1.0.1
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "ucvm_utils.h"
#include "ucvm_config.h"
#include "ucvm_proj_bilinear.h"

#include "linthurber.h"


/** The config of the model */
char *linthurber_config_string=NULL;
int linthurber_config_sz=0;

// Constants
/** The version of the model. */
const char *linthurber_version_string = "linthurber";

// Variables
/** Set to 1 when the model is ready for query. */
int linthurber_is_initialized = 0;

char linthurber_data_directory[128];

/** Configuration parameters. */
linthurber_configuration_t *linthurber_configuration;
/** Holds pointers to the velocity model data OR indicates it can be read from file. */
linthurber_model_t *linthurber_velocity_model;

/**
 * Initializes the linthurber plugin model within the UCVM framework. In order to initialize
 * the model, we must provide the UCVM install path and optionally a place in memory
 * where the model already exists.
 *
 * @param dir The directory in which UCVM has been installed.
 * @param label A unique identifier for the velocity model.
 * @return Success or failure, if initialization was successful.
 */
int linthurber_init(const char *dir, const char *label) {

    int tempVal = 0;
    char configbuf[512];
    double north_height_m = 0, east_width_m = 0, rotation_angle = 0;

    // Initialize variables.
    linthurber_configuration = calloc(1, sizeof(linthurber_configuration_t));
    linthurber_velocity_model = calloc(1, sizeof(linthurber_model_t));

    linthurber_config_string = calloc(LINTHURBER_CONFIG_MAX, sizeof(char));
    linthurber_config_string[0]='\0';
    linthurber_config_sz=0;

    // Configuration file location.
    sprintf(configbuf, "%s/model/%s/data/config", dir, label);

    // Read the linthurber_configuration file.
    if (linthurber_read_configuration(configbuf, linthurber_configuration) != SUCCESS)
        return FAIL;

    // Set up the data directory.
    sprintf(linthurber_data_directory, "%s/model/%s/data/%s/", dir, label, linthurber_configuration->model_dir);
    // Can we allocate the model, or parts of it, to memory. If so, we do.
    tempVal = linthurber_try_reading_model(linthurber_velocity_model);

    if (tempVal == SUCCESS) {
//        fprintf(stderr, "WARNING: Could not load model into memory. Reading the model from the\n");
//        fprintf(stderr, "hard disk may result in slow performance.\n");
    } else if (tempVal == FAIL) {
        linthurber_print_error("No model file was found to read from.");
        return FAIL;
    }

    // setup config_string 
    sprintf(linthurber_config_string,"config = %s\n",configbuf);
    linthurber_config_sz=1;

    // Let everyone know that we are initialized and ready for business.
    linthurber_is_initialized = 1;

    return SUCCESS;
}

/**
 * Queries linthurber at the given points and returns the data that it finds.
 *
 * @param points The points at which the queries will be made.
 * @param data The data that will be returned (Vp, Vs, rho, Qs, and/or Qp).
 * @param numpoints The total number of points to query.
 * @return SUCCESS or FAIL.
 */
int linthurber_query(linthurber_point_t *points, linthurber_properties_t *data, int numpoints) {

    linthurber_configuration_t *config=linthurber_configuration;
    linthurber_model_t *model=linthurber_velocity_model;
    
    int k, p;
    double x, y, z, depth_msl, depth_ratio;

    /* Query model */
    ucvm_point_t geo;      
    ucvm_point_t xy;
    ucvm_bilinear_t linthurber_proj;
    for (p = 0; p < numpoints; p++) {
        geo.coord[0]= points[p].longitude;
        geo.coord[1]= points[p].latitude;

        data[p].vp = -1.0;
        data[p].vs = -1.0;
        data[p].rho = -1.0;

        if (ucvm_bilinear_geo2xy(&linthurber_proj, &geo, &xy) == 0) {

          if (LINTHURBER_USE_DEM) {

              /* DEM elevation value */
              if (_linthurber_getval(x/config->spacing_dem,
                         y/config->spacing_dem,
                         0.0, LINTHURBER_DEM, 
                         &depth_msl) != SUCCESS) { continue; }

              depth_msl = (points[p].depth - depth_msl)/1000.0;
              for (k = 0; k < config->num_z; k++) {
                 if (config->depths_msl[k] >= depth_msl) { break; }
              }

              if (k == config->num_z) {
                 k = config->num_z - 1;
                 depth_ratio = 0.0;
                 z = k;
                 } else if (k == 0) {
                    depth_ratio = 0.0;
                    z = k;
                    } else {
                        depth_ratio = (depth_msl - config->depths_msl[k-1]) /
                                  (config->depths_msl[k] - config->depths_msl[k-1]); 
                        z = (k-1) + depth_ratio;
               }
       
               /* Vp value */
               _linthurber_getval(x / config->spacing_vp,
                              y / config->spacing_vp, z, LINTHURBER_VP, &(data[p].vp));
       
              /* Vs value */
               _linthurber_getval(x / config->spacing_vs,
                              y / config->spacing_vs, z, LINTHURBER_VS, &(data[p].vs));
              } else {
    
                  depth_msl = points[p].depth/1000.0;
                  for (k = 0; k < config->num_z; k++) {
                      if (config->depths_msl[k] >= depth_msl) { break; }
                  }
                  if (k == config->num_z) { k = config->num_z - 1; }
                  z = k;
   
                  /* Vp value */
                  _linthurber_getval(x / config->spacing_vp,
                            y / config->spacing_vp, z, LINTHURBER_VP, &(data[p].vp));
              
                  /* Vs value */
                  _linthurber_getval(x / config->spacing_vs,
                            y / config->spacing_vs, z, LINTHURBER_VS, &(data[p].vs));
          }

         /* Calculate density */
         if (data[p].vp > 0.0) { data[p].rho = _get_rho(data[p].vp); }
       }
    } // for loop
    return(SUCCESS);
}


int _linthurber_getval(double i, double j, double k, int prop,
                  double *val) {

  linthurber_configuration_t *config=linthurber_configuration;
  linthurber_model_t *model=linthurber_velocity_model;

  int i0, j0, k0;
  int a, b, c, x, y, z;
  int *dims = NULL;
  float *buf = NULL;
  double p[2][3];
  double q[2][2][2];

  *val = -1.0;

  i0 = (int)i;
  j0 = (int)j;
  k0 = (int)k;

  a = round(i);
  b = round(j);
  c = round(k);

  //fprintf(stderr, "ijk = %lf, %lf, %lf\n", i, j, k);

  switch (prop) {
  case LINTHURBER_DEM:
    dims = config->dem_dims;
    buf = model->dem;
    break;
  case LINTHURBER_VP:
    dims = config->vp_dims;
    buf = model->vp;
    break;
  case LINTHURBER_VS:
    dims = config->vs_dims;
    buf = model->vs;
   break;
  };

  /* Check if point falls outside of model region */
  if ((a < 0) || (b < 0) || (c < 0) || 
      (a >= dims[0]) || (b >= dims[1]) || (c >= dims[2])) {
    //fprintf(stderr, "a,b,c = %d, %d, %d\n", a, b, c);
    return(FAIL);
  }

  /* Values at corners of interpolation cube */
  for (z = 0; z < 2; z++) {
    for (y = 0; y < 2; y++) {
      for (x = 0; x < 2; x++) {

     a = i0 + x;
     b = j0 + y;
     c = k0 + z;

     if (a < 0) {
       a = 0;
     }
     if (b < 0) {
       b = 0;
     }
     if (c < 0) {
       c = 0;
     }
     if (a >= dims[0]) {
       a = dims[0] - 1;
     }
     if (b >= dims[1]) {
       b = dims[1] - 1;
     }
     if (c >= dims[2]) {
       c = dims[2] - 1;
     }

     q[z][y][x] = buf[c*dims[0]*dims[1] + b*dims[0] + a];
      }
    }
  }

  /* Corners of interpolation cube */
  for (b = 0; b < 2; b++) {
    for (a = 0; a < 3; a++) { 
      p[b][a] = (double)b;
    }
  }

  /* Trilinear interpolation from UCVM/src/ucvm/ucvm_utils.c */
  *val = interpolate_trilinear(i-i0, j-j0, k-k0, p, q);

  return(SUCCESS);
}


/* Density derived from Vp via Nafe-Drake curve, Brocher (2005) eqn 1. */
double _get_rho(double f) {
  double rho;

  /* Convert m to km */
  f = f / 1000.0;
  rho = f * (1.6612 - f * (0.4721 - f * (0.0671 - f * (0.0043 - f * 0.000106))));
  if (rho < 1.0) {
    rho = 1.0;
  }
  rho = rho * 1000.0;
  return(rho);
}


/**
 * Called when the model is being discarded. Free all variables.
 *
 * @return SUCCESS
 */
int linthurber_finalize() {

    if (linthurber_configuration) free(linthurber_configuration);

    if (linthurber_velocity_model) {
      if (linthurber_velocity_model->vp_status == 2)
          { free(linthurber_velocity_model->vp); }
      if (linthurber_velocity_model->vs_status == 2)
          { free(linthurber_velocity_model->vs); }
      if (linthurber_velocity_model->dem_status == 2)
          { free(linthurber_velocity_model->dem); }
      free(linthurber_velocity_model);
    }
    return SUCCESS;
}

/**
 * Returns the version information.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int linthurber_version(char *ver, int len)
{
  int verlen;
  verlen = strlen(linthurber_version_string);
  if (verlen > len - 1) {
    verlen = len - 1;
  }
  memset(ver, 0, len);
  strncpy(ver, linthurber_version_string, verlen);
  return 0;
}

/**
 * Returns the model config information.
 *
 * @param key Config key string to return.
 * @param sz number of config terms.
 * @return Zero
 */
int linthurber_config(char **config, int *sz)
{
  int len=strlen(linthurber_config_string);
  if(len > 0) {
    *config=linthurber_config_string;
    *sz=linthurber_config_sz;
    return SUCCESS;
  }
  return FAIL;
}


/**
 * Reads the linthurber_configuration file 
 *
 * @param file The linthurber_configuration file location on disk to read.
 * @param config The linthurber_configuration struct to which the data should be written.
 * @return Success or failure, depending on if file was read successfully.
 */
int linthurber_read_configuration(char *file, linthurber_configuration_t *config) {
    FILE *fp = fopen(file, "r");
    char key[40];
    char value[80];
    char line_holder[128];

    // If our file pointer is null, an error has occurred. Return fail.
    if (fp == NULL) {
        linthurber_print_error("Could not open the cs248_configuration file.");
        return FAIL;
    }

    // Read the lines in the linthurber_configuration file.
    while (fgets(line_holder, sizeof(line_holder), fp) != NULL) {
        if (line_holder[0] != '#' && line_holder[0] != ' ' && line_holder[0] != '\n') {
            sscanf(line_holder, "%s = %s", key, value);

            // Which variable are we editing?
            if (strcmp(key, "utm_zone") == 0) config->utm_zone = atoi(value);
            if (strcmp(key, "model_dir") == 0) sprintf(config->model_dir, "%s", value);

            if (strcmp(key, "spacing_vp") == 0) config->spacing_vp = atof(value);
            if (strcmp(key, "spacing_vs") == 0) config->spacing_vs = atof(value);
            if (strcmp(key, "spacing_dem") == 0) config->spacing_dem = atof(value);

            if (strcmp(key, "num_z") == 0) config->num_z = atoi(value);

            if (strcmp(key, "proj_xi") == 0) {
              _split4float(value, config->proj_xi,4);
            }
            if (strcmp(key, "proj_yi") == 0) {
              _split4float(value, config->proj_yi,4);
            }
            if (strcmp(key, "proj_dims") == 0) {
              _split4float(value, config->proj_dims,2);
            }
            if (strcmp(key, "depths_msl") == 0) {
              _split4float(value, config->depths_msl,LINTHURBER_MAX_Z_DIM);
            }
            if (strcmp(key, "grid_origin") == 0) {
              _split4float(value, config->grid_origin,2);
            }
            if (strcmp(key, "vp_origin") == 0) {
              _split4float(value, config->vp_origin,2);
              _split4float(value, config->vs_origin,2);
            }

            if (strcmp(key, "interpolation") == 0) { 
                config->interpolation=0;
                if (strcmp(value,"on") == 0) config->interpolation=1;
            }
        }
    }
    // calculated config setting
    config->dem_dims[0] = config->proj_dims[0] / config->spacing_dem + 1;
    config->dem_dims[1] = config->proj_dims[1] / config->spacing_dem + 1;
    config->dem_dims[2] = 1;

    config->vp_dims[0] = config->proj_dims[0] / config->spacing_vp + 1;
    config->vp_dims[1] = config->proj_dims[1] / config->spacing_vp + 1;
    config->vp_dims[2] = config->num_z;

    config->vs_dims[0] = config->proj_dims[0] / config->spacing_vs + 1;
    config->vs_dims[1] = config->proj_dims[1] / config->spacing_vs + 1;
    config->vs_dims[2] = config->num_z;


    fclose(fp);
    return SUCCESS;
}

int _split4float(char *str, double *val, int cnt) {
    // Declaration of delimiter
    const char s[4] = ",";
    char* tok;
 
    tok = strtok(str, s);
 
    // Checks for delimiter
    int idx=0;
    while (tok != 0 && idx < cnt) {
        val[idx]=atof(tok);
        tok = strtok(0, s);
        idx++;
    }
    return idx;
}

/**
 * Prints the error string provided.
 *
 * @param err The error string to print out to stderr.
 */
void linthurber_print_error(char *err) {
    fprintf(stderr, "An error has occurred while executing linthurber. The error was: %s\n",err);
    fprintf(stderr, "\n\nPlease contact software@scec.org and describe both the error and a bit\n");
    fprintf(stderr, "about the computer you are running linthurber on (Linux, Mac, etc.).\n");
}

/**
 * Check if the vp data is too big to be loaded internally (exceed maximum
 * allowable by a INT variable)
 *
 */
int _too_big_vp() {
    long max_size= (long) (linthurber_configuration->vp_dims[0] *
	                   linthurber_configuration->vp_dims[1] *
	                   linthurber_configuration->vp_dims[2]);
    long delta= max_size - INT_MAX;
    if( delta > 0) {
        return 1;
        } else {
            return 0;
    }
}

/**
 * Tries to read the model into memory.
 *
 * @param model The model parameter struct which will hold the pointers to the data either on disk or in memory.
 * @return 2 if all files are read to memory, SUCCESS if file is found but at least 1
 * is not in memory, FAIL if no file found.
 */
int linthurber_try_reading_model(linthurber_model_t *model) {
    int i,j,k;
    FILE *fp;
    char filename[UCVM_MAX_PATH_LEN];
    int num_read, retval;
    float dep, x, y, val;

    linthurber_configuration_t *config=linthurber_configuration;

    int vp_sz = (config->vp_dims[0] * config->vp_dims[1] * config->vp_dims[2]);
    int vs_sz = (config->vs_dims[0] * config->vs_dims[1] * config->vs_dims[2]);
    int dem_sz = (config->dem_dims[0] * config->dem_dims[1] * config->dem_dims[2]);

    /* Allocate buffers */
    model->vp = malloc(vp_sz * sizeof(float));
    model->vs = malloc(vs_sz * sizeof(float));
    model->dem = malloc(dem_sz * sizeof(float));

    if ((model->vp == NULL) || (model->vs == NULL) || (model->dem == NULL)) {
        fprintf(stderr, "Failed to allocate buffers Lin-Thurber model\n");
        return(FAIL);
    }

    /* initialize them to -1 */
    for (i = 0; i < vp_sz; i++) { model->vp[i] = -1.0; }
    for (i = 0; i < vs_sz; i++) { model->vs[i] = -1.0; }
    for (i = 0; i < dem_sz; i++) { model->dem[i] = 0.0; }

    /* Load Vp velocity file*/
    sprintf(filename, "%s/lin-thurber.vp", linthurber_data_directory);
    num_read = 0;
    fp = fopen(filename, "r");
    while (!feof(fp)) {
        retval = fscanf(fp, "%*f %*f %f %f %f %f %*f", &dep, &y, &x, &val);
        if (retval != EOF) {
            if (retval != 4) {
                fprintf(stderr, 
                    "Failed to read Lin-Thurber Vp file, line %d (parsed %d)\n",
                    num_read, retval);
                return(FAIL);
            }
            for (k = 0; k < config->num_z; k++) {
                if (config->depths_msl[k] >= dep) { break; }
            }
            /* Flip x and y axis */
            j = round((y + (-config->vp_origin[0])) 
                         * 1000.0 / config->spacing_vp);
            i = round((config->proj_dims[0]/1000.0 - 
                         (x + (-config->vp_origin[1]))) 
                         * 1000.0 / config->spacing_vp);
            if ((i < 0) || (j < 0) || (k < 0) || 
                               (i >= config->vp_dims[0]) || 
                               (j >= config->vp_dims[1]) || 
                               (k >= config->vp_dims[2])) {
              fprintf(stderr, "Invalid index %d,%d,%d calculated\n", i, j, k);
              return(FAIL);
            }
            //printf("x,y,z: %d, %d, %d\n", i, j, k);
            model->vp[k*config->vp_dims[0]*config->vp_dims[1] + 
                                    j*config->vp_dims[0] + i] = val * 1000.0;
            num_read++;
        }
    }
    fclose(fp);

    /* Load Vs velocity file */
    sprintf(filename, "%s/lin-thurber.vs", linthurber_data_directory);
    num_read = 0;
    fp = fopen(filename, "r");
    while (!feof(fp)) {
        retval = fscanf(fp, "%*f %*f %f %f %f %f %*f", &dep, &y, &x, &val);
        if (retval != EOF) {
            if (retval != 4) {
                fprintf(stderr, 
                    "Failed to read Lin-Thurber Vs file, line %d (parsed %d)\n",
		    num_read, retval);
                return(FAIL);
            }
            for (k = 0; k < config->num_z; k++) {
                if (config->depths_msl[k] >= dep) { break; }
            }
            /* Flip x and y axis */
            j = round((y + (-config->vs_origin[0])) 
                              * 1000.0 / config->spacing_vs);
            i = round((config->proj_dims[0]/1000.0 - 
                              (x + (-config->vs_origin[1]))) 
                              * 1000.0 / config->spacing_vs);
            if ((i < 0) || (j < 0) || (k < 0) || 
                     (i >= config->vs_dims[0]) || 
                     (j >= config->vs_dims[1]) || 
                     (k >= config->vs_dims[2])) {
               fprintf(stderr, "Invalid index %d,%d,%d calculated\n", i, j, k);
               return(FAIL);
            }
//printf("x,y,z: %d, %d, %d\n", i, j, k);
            model->vs[k*config->vs_dims[0]*config->vs_dims[1] + 
                             j*config->vs_dims[0] + i] = val * 1000.0;
            num_read++;
        }
    }
    fclose(fp);

    /* Load DEM file */
    sprintf(filename, "%s/lin-thurber.dem", linthurber_data_directory);
    fp = fopen(filename, "r");
    num_read = fread(model->dem, sizeof(float), dem_sz, fp);
    if (num_read != dem_sz) {
        fprintf(stderr, "Failed to read Lin-Thurber DEM file\n");
        return(FAIL);
    }

    /* Swap endian from LSB to MSB */
    if (system_endian() == UCVM_BYTEORDER_MSB) {
        for (i = 0; i < dem_sz; i++) {
              model->dem[i] = swap_endian_float(model->dem[i]);
        }
    }
    fclose(fp);

    model->vp_status = 2;
    model->vs_status = 2;
    model->dem_status = 2;

  return(SUCCESS);
}

// The following functions are for dynamic library mode. If we are compiling
// a static library, these functions must be disabled to avoid conflicts.
#ifdef DYNAMIC_LIBRARY

/**
 * Init function loaded and called by the UCVM library. Calls linthurber_init.
 *
 * @param dir The directory in which UCVM is installed.
 * @return Success or failure.
 */
int model_init(const char *dir, const char *label) {
    return linthurber_init(dir, label);
}

/**
 * Query function loaded and called by the UCVM library. Calls linthurber_query.
 *
 * @param points The basic_point_t array containing the points.
 * @param data The basic_properties_t array containing the material properties returned.
 * @param numpoints The number of points in the array.
 * @return Success or fail.
 */
int model_query(linthurber_point_t *points, linthurber_properties_t *data, int numpoints) {
    return linthurber_query(points, data, numpoints);
}

/**
 * Finalize function loaded and called by the UCVM library. Calls linthurber_finalize.
 *
 * @return Success
 */
int model_finalize() {
    return linthurber_finalize();
}

/**
 * Version function loaded and called by the UCVM library. Calls linthurber_version.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int model_version(char *ver, int len) {
    return linthurber_version(ver, len);
}

/**
 * Version function loaded and called by the UCVM library. Calls linthurber_config.
 *
 * @param config Config string to return.
 * @param sz number of config terms
 * @return Zero
 */
int model_config(char **config, int *sz) {
    return linthurber_config(config, sz);
}


int (*get_model_init())(const char *, const char *) {
        return &linthurber_init;
}
int (*get_model_query())(linthurber_point_t *, linthurber_properties_t *, int) {
         return &linthurber_query;
}
int (*get_model_finalize())() {
         return &linthurber_finalize;
}
int (*get_model_version())(char *, int) {
         return &linthurber_version;
}
int (*get_model_config())(char **, int*) {
    return &linthurber_config;
}


#endif
