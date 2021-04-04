import pystare
import netCDF4
import matplotlib.pyplot as plt
import cartopy.crs as ccrs
import numpy as np

def main():

    # open NetCDF file with STARE indices, get indices
    nc = netCDF4.Dataset("cbofs_STARE.nc")
    u_indices = nc.variables["u_STARE_indices"].__array__()

    # create arrays of vertices and center locations for STARE indices
    latsv, lonsv, lat_center, lon_center = pystare.to_vertices_latlon(
        u_indices.astype("int64") # not really ideal as we could lose prceision;
                                  # STARE encodes as uint64
    )
    
    # shift all values of longitude to be [-180.0, 180.0]
    latsv = np.array(latsv)
    lonsv = np.array(lonsv)
    lat_center = np.array(lat_center)
    lon_center = np.array(lon_center)
    lonsv[lonsv>180.] = lonsv[lonsv>180.] - 180.0
    lon_center[lon_center>180.] = lon_center[lon_center>180.] - 180.0
    
    # create our figure and axis
    fig = plt.figure(figsize=(16,16))
    ax = plt.axes(projection=ccrs.PlateCarree())
    ax.coastlines()
    
    plt.scatter(lon_center, lat_center, s=0.01)
    #ax.gridlines(draw_labels=True)
    ax.set_title("Center Points of STARE indices, CBOFS U-grid")
    plt.savefig("./figs/out.png")

    # close NetCDF resource
    nc.close()

if __name__ == "__main__":
    main()
