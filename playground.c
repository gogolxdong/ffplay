static void av_format_init_next(void)
{
    AVOutputFormat *prevout = NULL, *out;
    AVInputFormat *previn = NULL, *in;

    ff_mutex_lock(&avpriv_register_devices_mutex);

    for (int i = 0; (out = (AVOutputFormat *)muxer_list[i]); i++)
    {
        if (prevout)
            prevout->next = out;
        prevout = out;
    }

    if (outdev_list)
    {
        for (int i = 0; (out = (AVOutputFormat *)outdev_list[i]); i++)
        {
            if (prevout)
                prevout->next = out;
            prevout = out;
        }
    }

    for (int i = 0; (in = (AVInputFormat *)demuxer_list[i]); i++)
    {
        if (previn)
            previn->next = in;
        previn = in;
    }

    if (indev_list)
    {
        for (int i = 0; (in = (AVInputFormat *)indev_list[i]); i++)
        {
            if (previn)
                previn->next = in;
            previn = in;
        }
    }

    ff_mutex_unlock(&avpriv_register_devices_mutex);
}