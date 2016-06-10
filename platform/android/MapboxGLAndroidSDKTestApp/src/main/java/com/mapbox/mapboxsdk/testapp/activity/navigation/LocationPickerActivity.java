package com.mapbox.mapboxsdk.testapp.activity.navigation;

import android.graphics.PointF;
import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Gravity;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import com.mapbox.mapboxsdk.annotations.Marker;
import com.mapbox.mapboxsdk.annotations.MarkerOptions;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback;
import com.mapbox.mapboxsdk.testapp.R;
import com.mapbox.services.commons.ServicesException;
import com.mapbox.services.commons.models.Position;
import com.mapbox.services.commons.utils.TextUtils;
import com.mapbox.services.geocoding.v5.GeocodingCriteria;
import com.mapbox.services.geocoding.v5.MapboxGeocoding;
import com.mapbox.services.geocoding.v5.models.GeocodingFeature;
import com.mapbox.services.geocoding.v5.models.GeocodingResponse;
import java.util.List;
import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;

public class LocationPickerActivity extends AppCompatActivity {

    private static final String TAG = "LocationPickerActivity";

    private MapView mapView;
    private MapboxMap mapboxMap;

    private ImageView dropPinView = null;
    private Marker resultsPin = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_location_picker);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setDisplayShowHomeEnabled(true);
        }

        mapView = (MapView) findViewById(R.id.mapView);
        mapView.onCreate(savedInstanceState);

        mapView.getMapAsync(new OnMapReadyCallback() {
            @Override
            public void onMapReady(MapboxMap map) {
                mapboxMap = map;
            }
        });

        dropPinView = new ImageView(this);
        dropPinView.setImageResource(R.drawable.default_marker);
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.CENTER);
        dropPinView.setLayoutParams(params);
        mapView.addView(dropPinView);

        final Button selectLocation = (Button) findViewById(R.id.selectLocationButton);
        selectLocation.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Log.i(TAG, "Location Selected!");
                        if (mapboxMap != null) {
                            Log.i(TAG, "dropPinView: height = " + dropPinView.getHeight() + "; width = " + dropPinView.getWidth() + "; left = " + dropPinView.getLeft() + "; bottom = " + dropPinView.getBottom());
                            float[] coords = getDropPinTipCoordinates();
                            LatLng latLng = mapboxMap.getProjection().fromScreenLocation(new PointF(coords[0], coords[1]));
                            Log.i(TAG, "location:  x = " + coords[0] + "; y = " + coords[1] + "; latLng = " + latLng);
                            geocode(latLng);
                        }
                    }
                }
        );
    }

    private float[] getDropPinTipCoordinates() {
        float x = dropPinView.getLeft() + (dropPinView.getWidth() / 2);
        float y = dropPinView.getBottom();

        return new float[] {x, y};
    }

    private void geocode(final LatLng point) {
        try {
            MapboxGeocoding client = new MapboxGeocoding.Builder()
                    .setAccessToken(getString(R.string.mapbox_access_token))
                    .setCoordinates(Position.fromCoordinates(point.getLongitude(), point.getLatitude()))
                    .setType(GeocodingCriteria.TYPE_ADDRESS)
                    .build();

            client.enqueueCall(new Callback<GeocodingResponse>() {
                @Override
                public void onResponse(Call<GeocodingResponse> call, Response<GeocodingResponse> response) {

                    List<GeocodingFeature> results = response.body().getFeatures();
                    if (results.size() > 0) {
                        String address = results.get(0).getPlaceName();
                        Log.i(TAG, "address " + address);
                        showAddressPin(address);
                    } else {
                        Log.i(TAG, "No results for search.");
                        showAddressPin(null);
                    }
                }

                @Override
                public void onFailure(Call<GeocodingResponse> call, Throwable t) {
                    Log.e(TAG, "Geocoding Failure: " + t.getMessage());
                    showAddressPin(null);
                }
            });
        } catch (ServicesException e) {
            Log.e(TAG, "Error geocoding: " + e.toString());
            e.printStackTrace();
        }
    }

    private void showAddressPin(final String address) {

        if (resultsPin != null) {
            mapboxMap.removeAnnotation(resultsPin);
        }

        if (TextUtils.isEmpty(address)) {
            // Set back to search mode
            resultsPin = null;
            dropPinView.setVisibility(View.VISIBLE);

            return;
        }

        dropPinView.setVisibility(View.GONE);

        float[] coords = getDropPinTipCoordinates();
        LatLng latLng = mapboxMap.getProjection().fromScreenLocation(new PointF(coords[0], coords[1]));

        MarkerOptions markerOptions = new MarkerOptions().title(address).position(latLng);
        resultsPin = mapboxMap.addMarker(markerOptions);
        mapboxMap.selectMarker(resultsPin);
    }

    @Override
    public void onResume() {
        super.onResume();
        mapView.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        mapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mapView.onSaveInstanceState(outState);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mapView.onDestroy();
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
        mapView.onLowMemory();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                onBackPressed();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

}
