package org.example;

import java.io.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Scanner;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;

public class PlacesInterface {
    private static HashMap<String, String> api_keys;
    public void start() {
        api_keys = new HashMap<>();
        System.out.println("Введите название места: ");
        Scanner scanner = new Scanner(System.in);
        String place = scanner.nextLine();

        File file = new File(
                "api_keys"
        );

        try {
            BufferedReader br;
            br = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
            String string_api;
            while ((string_api = br.readLine()) != null) {
                api_keys.put(string_api.split(" ")[0], string_api.split(" ")[1]);
            }

            Places places = new Places(api_keys);
            ArrayList<String> placeNames = places.getPlaces(place);
            for (String name: placeNames) {
                System.out.println(name);
            }
            int count = placeNames.size();
            System.out.println("Выберите место из предложенных (от 1 до " + count + "): ");
            int chosenPlace = scanner.nextInt();
            while (chosenPlace < 1 || chosenPlace > count) {
                System.out.println("Error");
                chosenPlace = scanner.nextInt();
            }

            Integer finalChosenPlace = chosenPlace;
            CompletableFuture<ArrayList<String>> placesAndDescriptionEvent = CompletableFuture.supplyAsync(
                    () -> {
                        try {
                            return places.placesInArea(finalChosenPlace);
                        } catch (IOException e) {
                            throw new RuntimeException(e);
                        }
                    }
            );
            CompletableFuture<String> weatherEvent = CompletableFuture.supplyAsync(
                    () -> {
                        try {
                            return places.getWeather(finalChosenPlace);
                        } catch (IOException e) {
                            throw new RuntimeException(e);
                        }
                    }
            );

            CompletableFuture<Object> combinedFuture = weatherEvent.thenCombine(placesAndDescriptionEvent,
                    (weather, placesAndDescription) -> {
                        System.out.println("Погода: \n" + weather);
                        System.out.println("Интересные места: \n" + placesAndDescription);
                        return null;
                    });

            try {
                combinedFuture.get();
            } catch (InterruptedException | ExecutionException e) {
                throw new RuntimeException(e);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
