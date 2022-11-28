#include <gst/gst.h>

int
main (int argc, char *argv[])
{
  GstElement *pipeline, *source, *filter_vertigo, *videoconvert, *sink;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  /*
    * gst_element_factory_make() is a convenience function that creates an element
    * from a factory.
    * The first argument is the name of the factory to use, the second is the name
  */
  source = gst_element_factory_make ("videotestsrc", "source");
  filter_vertigo = gst_element_factory_make("vertigotv", "vertigo-filter");
  videoconvert = gst_element_factory_make("videoconvert", "video-convert");
  sink = gst_element_factory_make ("autovideosink", "sink");

  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");

  if (!pipeline || !source || !filter_vertigo || !videoconvert || !sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), source, filter_vertigo, videoconvert, sink, NULL);
  if (gst_element_link (source, filter_vertigo) != TRUE) {
    g_printerr ("Elements source and filter_vertigo could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (filter_vertigo, videoconvert) != TRUE) {
    g_printerr ("Elements filter_vertigo and videoconvert could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (videoconvert, sink) != TRUE) {
    g_printerr ("Elements videoconvert and sink could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  /* Modify the source's properties */
  g_object_set (source, "pattern", 0, NULL);

  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg =
      gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
      GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  /* Parse message */
  if (msg != NULL) {
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE (msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error (msg, &err, &debug_info);
        g_printerr ("Error received from element %s: %s\n",
            GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n",
            debug_info ? debug_info : "none");
        g_clear_error (&err);
        g_free (debug_info);
        break;
      case GST_MESSAGE_EOS:
        g_print ("End-Of-Stream reached.\n");
        break;
      default:
        /* We should not reach here because we only asked for ERRORs and EOS */
        g_printerr ("Unexpected message received.\n");
        break;
    }
    gst_message_unref (msg);
  }

  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}