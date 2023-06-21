#include <json-glib/json-glib.h>
#include <libsoup/soup.h>

#include <stdio.h>

void list_artists(JsonArray* array, guint index_, JsonNode* element_node, gpointer user_data)
{
    int NumOfArtists = json_array_get_length(array);
    JsonObject *ArtistObj = json_array_get_object_element(array, index_);

    printf("'%s'", json_object_get_string_member(ArtistObj, "name"));

    if (index_ < (NumOfArtists - 1))
        g_print(", ");
}

gboolean on_send_heartbeat(gpointer data)
{
    SoupWebsocketConnection *conn = data;
    g_info("Heartbeat sent - ");

    soup_websocket_connection_send_text(conn, "{ \"op\": 9 }");
    return TRUE;
}

static void on_close(SoupWebsocketConnection *conn, gpointer data)
{
    soup_websocket_connection_close(conn, SOUP_WEBSOCKET_CLOSE_NORMAL, NULL);
    g_print("WebSocket connection closed\n");
}

static void on_message(SoupWebsocketConnection *conn, gint type, GBytes *message, gpointer data)
{
    if (type == SOUP_WEBSOCKET_DATA_TEXT) {
        JsonObject *MsgObj, *BodyData, *SongObj;
        JsonArray *ArtistsArray;
        gsize sz;

        JsonNode *MsgData = json_from_string(g_bytes_get_data(message, &sz), NULL);
        MsgObj = json_node_get_object(MsgData);

        int opcode = json_object_get_int_member(MsgObj, "op");
        
        switch (opcode)
        {
        case 0:
            BodyData = json_object_get_object_member(MsgObj, "d");

            g_print("%s\n", json_object_get_string_member(BodyData, "message"));
            unsigned int HeartbeatInterval = json_object_get_int_member(BodyData, "heartbeat");

            g_timeout_add(HeartbeatInterval, (GSourceFunc)on_send_heartbeat, conn);
            break;
        case 1:
            BodyData = json_object_get_object_member(MsgObj, "d");

            SongObj = json_object_get_object_member(BodyData, "song");
            ArtistsArray = json_object_get_array_member(SongObj, "artists");
            
            printf("\033[KPlaying '%s'\n", json_object_get_string_member(SongObj, "title"));
            g_print("\033[KBy ");

            json_array_foreach_element(ArtistsArray, (JsonArrayForeach)list_artists, NULL);
            g_print("\033[1A\r");

            break;
        case 10:
            g_info("acknowledged!\n");
            break;
        default:
            break;
        }

    } else if (type == SOUP_WEBSOCKET_DATA_BINARY) {
        g_print("Received binary data (not shown)\n");
    } else {
        g_print("Invalid data type: \"%d\"\n", type);
    }
}

static void on_connection(SoupSession *session, GAsyncResult *res, gpointer data)
{
    SoupWebsocketConnection *conn;
    GError *err = NULL;

    conn = soup_session_websocket_connect_finish(session, res, &err);
    if (err) {
        g_print("Error: %s\n", err->message);
        g_error_free(err);
        return;
    }

    g_signal_connect(conn, "message", G_CALLBACK(on_message), NULL);
}

int main(int argc, char *argv[])
{
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    SoupSession *WebSocketSession = soup_session_new();
    SoupMessage *msg = soup_message_new(SOUP_METHOD_GET, "wss://listen.moe/gateway_v2");

    soup_session_websocket_connect_async(WebSocketSession,
        msg,
        NULL, NULL,
        G_PRIORITY_DEFAULT,
        NULL, 
        (GAsyncReadyCallback)on_connection,
        NULL
    );

    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    g_object_unref(msg);
    g_object_unref(WebSocketSession);

    return 0;
}