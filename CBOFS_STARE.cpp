#include <netcdf.h> /* all NC*, nc* stuff */
#include <iostream> /* pretty much only cout */
#include <vector>   /* Vector */
#include "STARE.h"  /* All STARE */

#ifndef HAVE_OPENMP
#define HAVE_OPENMP 1
#endif

/* Open up a CBOFS NetCDF file and compute the STARE indices.
 * This uses the netcdf-c API directly, so we'll deal with
 * C-style arrays for a bit. Likely would move to dynamically
 * allocated arrays (std::array?) in production environment.
 */

/* In a production workflow, we'd have these passed in via a
 * configuration option
 */

/* CBOFS has 332x291 points "in the horizontal"
 * https://tidesandcurrents.noaa.gov/ofs/cbofs/cbofs_info.html
 */
#define ETA_U 291
#define ETA_V 290
#define XI_U  331
#define XI_V  332

int main(int argc, char **argv)
{

    /* some constants that don't (seem) to change */
    int ncid;

    /* initialize variable IDs with -1 */
    int LAT_U_ID = -1;
    int LON_U_ID = -1;
    int LAT_V_ID = -1;
    int LON_V_ID = -1;

    /* static allocation of lat_u, lon_u, lat_v, lon_v containers */
    /* nc_get_var_* gets copies data into contiguous memory slots */
    double lat_u[ETA_U * XI_U];
    double lon_u[ETA_U * XI_U];
    double lat_v[ETA_V * XI_V];
    double lon_v[ETA_V * XI_V];

    /* Dimension id containers, variable id containers
     * for the new NetCDF file containing the computed STARE values;
     * the U and V dimensions are, like above, the product of the
     * respective eta and xi dimensions (we'll define these
     * in the new file later on)
     */
    int u_dim_id, v_dim_id, u_STARE_id, v_STARE_id;
    int u_dims[1], v_dims[1];

    /* STARE object, index containers */
    /* TODO define build resolution (27) above */
    STARE stare_obj(27, STARE_HARDWIRED_BUILD_LEVEL_MAX);
    STARE_ArrayIndexSpatialValues u_STARE;
    STARE_ArrayIndexSpatialValues v_STARE;

    /* open the NetCDF file */
    int status; /* for checking function returns/errors */
    status = nc_open("nos.cbofs.fields.f001.20210403.t00z.nc", NC_NOWRITE, &ncid);
    if (status != NC_NOERR)
    {
        std::cout << "ERROR OPENING FILE! " << status << std::endl;
        return status;
    }
    std::cout << "ncdid: " << ncid << std::endl;

    /* pull out the latitude and longitude variable ID numbers, assigns value to ptr*/
    status = nc_inq_varid(ncid, "lat_u", &LAT_U_ID);
    if (status != NC_NOERR) {std::cout << "ERROR GETTING 'lat_u'!" << std::endl; return status;}
    status = nc_inq_varid(ncid, "lon_u", &LON_U_ID);
    if (status != NC_NOERR) {std::cout << "ERROR GETTING 'lon_u'!" << std::endl; return status;}
    status = nc_inq_varid(ncid, "lat_v", &LAT_V_ID);
    if (status != NC_NOERR) {std::cout << "ERROR GETTING 'lat_v'!" << std::endl; return status;}
    status = nc_inq_varid(ncid, "lon_v", &LON_V_ID);
    if (status != NC_NOERR) {std::cout << "ERROR GETTING 'lon_v'!" << std::endl; return status;}

    /* pull out the arrays of the variables themselves;
     * value gets copied using memcpy to contiguous array
     */
    status = nc_get_var_double(ncid, LAT_U_ID, lat_u);
    if (status != NC_NOERR) {std::cout << "ERROR reading lat_u data: " << status << std::endl; return status;}

    status = nc_get_var_double(ncid, LON_U_ID, lon_u);
    if (status != NC_NOERR) {std::cout << "ERROR reading lon_u data: " << status << std::endl; return status;}

    status = nc_get_var_double(ncid, LAT_V_ID, lat_v);
    if (status != NC_NOERR) {std::cout << "ERROR reading lat_v data: " << status << std::endl; return status;}

    status = nc_get_var_double(ncid, LON_V_ID, lon_v);
    if (status != NC_NOERR) {std::cout << "ERROR reading lon_v data: " << status << std::endl; return status;}

    /* we're done with this NetCDF file - close it */
    status = nc_close(ncid);
    if (status != NC_NOERR)
    {
        std::cout << "ERROR CLOSING FILE! " << status << std::endl;
        return status;
    }

    /* compute STARE index by iterating over 1D arrays
     * as if they were 2D. We'll proceed with U-values first,
     * followed by V.
     */

    #pragma omp for
    for (int i=0; i<ETA_U; ++i) /* "rows" */
    {
        for (int j=0; j<XI_U; ++j) /* "columns" */
        {
            u_STARE.push_back(stare_obj.ValueFromLatLonDegrees(
                lat_u[i + ETA_U * j],
                lon_u[i + ETA_U * j]
            ));
        }
    }

    #pragma omp for
    for (int i=0; i<ETA_V; ++i) /* "rows" */
    {
        for (int j=0; j<XI_V; ++j) /* "columns" */
        {
            v_STARE.push_back(stare_obj.ValueFromLatLonDegrees(
                lat_v[i + ETA_V * j],
                lon_v[i + ETA_V * j]
            ));
        }
    }

    // write STARE indices to a separate NetCDF file
    ncid = -1; // "clear" the previous file ID

    // create new NetCDF file, overwriting if it exists already
    if ((status = nc_create("cbofs_STARE.nc", NC_CLOBBER|NC_NETCDF4, &ncid)))
        { std::cout << "ERROR creating new file: " << status << std::endl; }

    // define the dimensions
    if ((status = nc_def_dim(ncid, "u_dim", ETA_U*XI_U, &u_dim_id)))
        { std::cout << "ERROR defining dim: " << status << std::endl; }
    if ((status = nc_def_dim(ncid, "v_dim", ETA_V*XI_V, &v_dim_id)))
        { std::cout << "ERROR defining dim: " << status << std::endl; }

    // this array will be passed to the variable creator function
    u_dims[0] = u_dim_id;
    v_dims[0] = v_dim_id;

    // define the variables; each has only one dimension
    // corresponding to the ones defined at the beginning
    if ((status = nc_def_var(
        ncid, "u_STARE_indices", NC_UINT64, 1, u_dims, &u_STARE_id)))
        { std::cout << "ERROR defining variable: " << status << std::endl; exit(status);}

    if ((status = nc_def_var(
        ncid, "v_STARE_indices", NC_UINT64, 1, v_dims, &v_STARE_id)))
        { std::cout << "ERROR defining variable: " << status << std::endl; exit(status);}

    // sweet, ready to write data to the file; end "define" mode
    if ((status = nc_enddef(ncid))) { std::cout << "ERROR ending define mode: " << status << std::endl; }

    // write the variable data to the file
    if ((status = nc_put_var(
        ncid, u_STARE_id, u_STARE.data())))
        { std::cout << "ERROR writing u_STARE: " << status << std::endl; }

    if ((status = nc_put_var(
        ncid, v_STARE_id, v_STARE.data())))
        { std::cout << "ERROR writing v_STARE: " << status << std::endl; }

    // close our newly writeen file
    if ((status = nc_close(ncid))) { std::cout << "ERROR closing file: " << status << std::endl; }
    

    return 0;
}
