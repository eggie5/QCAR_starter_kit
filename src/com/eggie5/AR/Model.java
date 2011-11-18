
package com.eggie5.AR;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;

import java.io.BufferedInputStream;
import java.io.IOException;


import android.content.res.AssetManager;

import org.apache.commons.lang3.ArrayUtils;
import org.codehaus.jackson.JsonFactory;
import org.codehaus.jackson.JsonParser;
import org.codehaus.jackson.JsonToken;

import android.util.Log;

public class Model {
    private float[] vertices;

    private float[] normals;

    private float[] texture_coords;

	public static Model loadModelFromApk(String fileName,  AssetManager assets)
    {
        InputStream inputStream = null;
        try
        {
            inputStream = assets.open(fileName, AssetManager.ACCESS_BUFFER);
			return Model.from_json(inputStream);
		}
        catch (IOException e)
        {
            DebugLog.LOGE("Failed to load model '" + fileName + "' from APK.");
            DebugLog.LOGI(e.getMessage());
            return null;
        }
	}

    public static Model from_json(InputStream model_io_stream) throws java.io.IOException {
        long startLocal = System.currentTimeMillis();

        Hashtable<String, List<Float>> buffers = new Hashtable<String, List<Float>>();

        // load file
        JsonFactory sJsonFactory = new JsonFactory();
        JsonParser p = sJsonFactory.createJsonParser(model_io_stream);

        List<Float> buffer;
        p.nextToken();
        while (p.nextToken() != JsonToken.END_OBJECT) {
            String key = p.getCurrentName();
            DebugLog.LOGD("key: " + key);
            p.nextToken();

            // found vertices:
            // now find array start
            buffer = new ArrayList<Float>();
            while (p.nextToken() != JsonToken.END_ARRAY) {
                buffer.add(p.getFloatValue());
            }
            buffers.put(key, buffer);
        }
        p.close();

        Log.d("************Model:from_json", "Deseralization took: " + (System.currentTimeMillis() - startLocal) + "ms");

        startLocal = System.currentTimeMillis();
        Model m = new Model();
        m.vertices = ArrayUtils.toPrimitive(buffers.get("vertices").toArray(new Float[0]), 0.0f);
        m.normals = ArrayUtils.toPrimitive(buffers.get("normals").toArray(new Float[0]), 0.0f);
        m.texture_coords = ArrayUtils.toPrimitive(buffers.get("texture_coords").toArray(new Float[0]), 0.0f);

        Log.d("************Model:from_json", "Deseralization took: " + (System.currentTimeMillis() - startLocal) + "ms");

        return m;

    }

    // // deseralize json to Model object
    // public static Model from_json(String json) {
    // 	// deseralize json
    // 	Gson gson = new Gson();
    // 	// String big_string=new String(content, ASCII);
    // 	Model m = gson.fromJson(json, Model.class);
    // 	return m;
    // }

    public float[] getVertices() {
        return this.vertices;
    }

    public float[] getNormals() {
        return this.normals;
    }

    public float[] getTextureCoords() {
        return this.texture_coords;
    }
}