package io.joynr.bounceproxy.service;

/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2017 BMW Car IT GmbH
 * %%
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * #L%
 */

import io.joynr.bounceproxy.attachments.AttachmentStorage;
import io.joynr.messaging.bounceproxy.LongPollingMessagingDelegate;

import java.io.IOException;
import java.io.InputStream;
import java.net.URI;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.Consumes;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.UriInfo;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import joynr.JoynrMessage;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.inject.Inject;
import com.sun.jersey.multipart.FormDataParam;

@Path("/channels/{ccid: [A-Z,a-z,0-9,_,\\-,\\.]+}/messageWithAttachment")
public class AttachmentSenderService {

    private static final Logger log = LoggerFactory.getLogger(AttachmentSenderService.class);

    private final AttachmentStorage attachmentStorage;
    private final ObjectMapper objectMapper;
    private final LongPollingMessagingDelegate longPollingDelegate;

    @Context
    UriInfo ui;

    @Context
    HttpServletRequest request;

    @Inject
    public AttachmentSenderService(AttachmentStorage attachmentStorage,
                                   ObjectMapper objectMapper,
                                   LongPollingMessagingDelegate longPollingDelegate) {
        this.attachmentStorage = attachmentStorage;
        this.objectMapper = objectMapper;
        this.longPollingDelegate = longPollingDelegate;
    }

    @POST
    @Produces(MediaType.APPLICATION_JSON)
    @Consumes(MediaType.MULTIPART_FORM_DATA)
    public Response postMessageWithAttachment(@PathParam("ccid") String ccid,
                                              @FormDataParam("wrapper") String serializedMessage,
                                              @FormDataParam("attachment") InputStream attachment) {

        try {
            JoynrMessage message = objectMapper.readValue(serializedMessage, JoynrMessage.class);
            log.debug("Message with attachment received! Expiry Date : " + message.getExpiryDate());
            int bytesRead = 0;
            int bytesToRead = 1024;
            byte[] input = new byte[bytesToRead];
            while (bytesRead < bytesToRead) {
                int result = attachment.read(input, bytesRead, bytesToRead - bytesRead);
                if (result == -1)
                    break;
                bytesRead += result;
            }
            attachmentStorage.put(message.getId(), input);

            log.debug("Uploaded attachment : " + new String(input, 0, Math.min(50, input.length), "UTF-8"));

            // post message
            try {
                String msgId = message.getId();
                log.debug("******POST message {} to cluster controller: {}", msgId, ccid);
                log.trace("******POST message {} to cluster controller: {} extended info: \r\n {}", ccid, message);

                // the location that can be queried to get the message
                // status
                // TODO REST URL for message status?
                String path = longPollingDelegate.postMessage(ccid, message);

                URI location = ui.getBaseUriBuilder().path(path).build();
                // return the message status location to the sender.
                return Response.created(location).header("msgId", msgId).build();

            } catch (WebApplicationException e) {
                log.error("Invalid request from host {}", request.getRemoteHost());
                throw e;
            } catch (Exception e) {
                log.error("POST message for cluster controller: error: {}", e.getMessage());
                throw new WebApplicationException(e);
            }

        } catch (IOException e) {
            log.error("POST message for cluster controller: error: {}", e.getMessage(), e);
            return Response.serverError().build();
        }
    }
}
