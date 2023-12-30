package org.example;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

import java.io.IOException;

public class Utils {

    public static JsonObject getJson(String url) throws IOException {
        OkHttpClient client = new OkHttpClient();
        Request request = new Request.Builder()
                .url(url)
                .get()
                .build();
        Response response = client.newCall(request).execute();

        Gson gson = new Gson();
        assert response.body() != null;
        return gson.fromJson(response.body().string(), JsonObject.class);
    }
}
