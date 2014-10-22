/**
 * This is a proof of concept piece of code that is designed
 * to model the reworking of Lumails attachment primitves.
 *
 * Currently there are two parts of src/message.cc that deal with
 * attachments
 *
 *   CMessage::attachments()
 *
 *   CMessage::save_attachment(int offset)
 *
 * The former iterates over the mail and returns a vector of attachment-names,
 * the latter iterates over the mail, counting attachments, and saving the
 * single one specified by offset.
 *
 * It seems obvious that we should only have one piece of code, and instead
 * we should store a vector of attachments which are parsed on demand
 * (and cached):
 *
 *   CAttachment( filename, data, data_len )
 *
 * Given the potential complexity, and the issue with inline attachments
 * highlighted by #186 we should be careful and thus this code is born.
 *
 * Steve
 * --
 */


#include <string>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>
#include <assert.h>
#include <vector>


#include "utfstring.h"


/**
 * Stub class to hold attachments.
 */
class CAttachment
{
public:

    /**
     * Constructor
     */
    CAttachment(UTFString name, void * body, size_t sz ) {

        /**
         * Save the name away.
         */
        m_name = name;

        /**
         * Save the data away
         */
        m_data = (void *)malloc( sz );
        memcpy( m_data, body, sz );

        m_size = sz;
    };

    ~CAttachment()
    {
        if ( m_data != NULL )
        {
            free( m_data );
            m_data = NULL;
        }
    }
    /**
     * Return the (file)name of the attachment.
     */
    UTFString name() { return m_name ; }

    /**
     * Return the body of the attachment.
     */
    void *body() { return m_data ; }

    /**
     * Return the size of the attachment.
     */
    size_t size() { return m_size ; }

private:
    UTFString m_name;
    void    * m_data;
    size_t    m_size;
};



/**
 * Global pointer to the message.
 */
GMimeMessage *m_message;

/**
 * Given a single email message we want to parse than and populate
 * a vector of CAttachment objects.
 *
 * This will then be ruturned.
 */
std::vector<CAttachment*> handle_mail( const char *filename )
{
    std::vector<CAttachment *> results;

    std::cout << std::endl;
    std::cout << "Handling input message: " << filename << std::endl;



/**
 ** Standard setup to match CMessage ..
 **/

    GMimeParser *parser;
    GMimeStream *stream;
    int m_fd = open( filename, O_RDONLY, 0);

    if ( m_fd < 0 )
    {
        std::cerr << "Failed to open file" << std::endl;
        exit(1);
    }

    stream = g_mime_stream_fs_new (m_fd);

    parser = g_mime_parser_new_with_stream (stream);
    g_object_unref (stream);

    m_message = g_mime_parser_construct_message (parser);

    if ( m_message == NULL )
    {
        std::cerr << "Failed to construct message" << std::endl;
        exit(1);
    }

    g_object_unref (parser);


/**
 ** REAL START OF TEST CODE
 **
 **/

    GMimePartIter *iter =  g_mime_part_iter_new ((GMimeObject *) m_message);
    assert(iter != NULL);

    /**
     * Iterate over the message.
     */
    do
    {
        GMimeObject *part  = g_mime_part_iter_get_current (iter);
        if  (GMIME_IS_MULTIPART( part ) )
            continue;

        /**
         * Name of the attachment, if we found one.
         */
        char *aname = NULL;

        /**
         * Size of the attachment, if we found one.
         */
        size_t asize = 0;

        /**
         * Attachment content, if we found one.
         */
        char *adata = NULL;


        /**
         * Get the content-disposition, so that we can determine
         * if we're dealing with an attachment, or an inline-part.
         */
        GMimeContentDisposition *disp = NULL;
        disp = g_mime_object_get_content_disposition (part);

        if ( ( disp != NULL ) &&
             ( !g_ascii_strcasecmp (disp->disposition, "attachment") ) )
        {
            /**
             * Attempt to get the filename/name.
             */
            aname = (char *)g_mime_object_get_content_disposition_parameter(part, "filename");
            if ( aname == NULL )
                aname = (char *)g_mime_object_get_content_type_parameter(part, "name");


            /**
             * Did that work?
             */
            if ( aname == NULL )
            {
                std::cout << "\tAttachment has no name." << std::endl;
            }
            else
            {
                std::cout << "\tAttachment has name : " << aname << std::endl;
            }

        }
        else
        {
            if ( disp != NULL && disp->disposition != NULL )
                std::cout << "\tInline part with name: " << disp->disposition << std::endl;
            else
                std::cout << "\tAnonymous inline part."  << std::endl;


        }


        /**
         * Get the attachment data.
         */
        GMimeStream *mem = g_mime_stream_mem_new();

 if (GMIME_IS_MESSAGE_PART (part))
         {
             GMimeMessage *msg = g_mime_message_part_get_message (GMIME_MESSAGE_PART (part));

             g_mime_object_write_to_stream (GMIME_OBJECT (msg), mem);
         }
         else
         {
             GMimeDataWrapper *content = g_mime_part_get_content_object (GMIME_PART (part));

             g_mime_data_wrapper_write_to_stream (content, mem);
         }

         /**
          * NOTE: by setting the owner to FALSE, it means unreffing the
          * memory stream won't free the GByteArray data.
          */
         g_mime_stream_mem_set_owner (GMIME_STREAM_MEM (mem), FALSE);

         GByteArray *res =  g_mime_stream_mem_get_byte_array (GMIME_STREAM_MEM (mem));

         /**
          * The actual data from the array, and the size of that data.
          */
         adata = (char *)res->data;
         size_t len = (res->len);

         g_object_unref (mem);

         /**
          * Save the resulting attachment to the array we return.
          */
         if ( adata != NULL )
         {
             CAttachment *foo = new CAttachment( aname ? aname :  "inline-part-%d",
                                                 (void *)adata,(size_t ) len );
             results.push_back(foo);
             std::cout << "\tAdded part to return value" << std::endl;
        }
        else
        {
            std::cout << "\tFailed to add part to return value" << std::endl;
        }

    }

    while (g_mime_part_iter_next (iter));

    g_mime_part_iter_free (iter);

/**
 ** Standard cleanup to match CMessage ..
 **/
    if ( m_message != NULL )
    {
        g_object_unref( m_message );
        m_message = NULL;
    }

    return( results );
}


/**
 * For each argument show the attachments in the specified message.
 */
int main( int argc, char *argv[] )
{
    g_mime_init(0);

    for( int i = 1; i <argc; i++ )
    {

        /**
         * Parse the given mail-file.
         */
        std::vector<CAttachment *> result =  handle_mail( argv[i] );


        /**
         * Show the initial results.
         */
        std::cout << "Parsing has completed" << std::endl;

        std::cout << "We received " << result.size()
                  << " attachment(s)." << std::endl;

        /**
         * Iterate over each detected attachment.
         */
        int c = 0;
        for (CAttachment *cur : result)
        {
            /**
             * Show some details.
             */
            std::cout << "\tNAME: " << cur->name()
                      << " size: " << cur->size()
                      << std::endl;


            fprintf(stderr, "PART %d: %s\n", c , (char *)cur->body() );
            c += 1;
            delete(cur);
        }
    }


    /**
     * Cleanup.
     */
    g_mime_shutdown();

    return ( 0 );

}
