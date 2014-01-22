package io.joynr.messaging.service;

/*
 * #%L
 * joynr::java::messaging::bounceproxy-controller-service
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
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

import io.joynr.messaging.info.BounceProxyStatus;
import io.joynr.messaging.info.PerformanceMeasures;

import java.net.URI;
import java.util.List;
import java.util.Map;

import javax.ws.rs.Consumes;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.GenericEntity;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import com.google.inject.Inject;

/**
 * RESTful web service to control bounce proxy instances.
 * 
 * It offers methods for bounce proxies to report their status and also
 * performance measures. Based on these reports the bounce proxy controller will
 * be able to determine whether bounce proxy instances can handle more channels.
 * 
 * @author christina.strobel
 * 
 */
@Path("/controller/bounceproxies")
public class MonitoringServiceRestAdapter {

    @Inject
    private MonitoringService monitoringService;

    /**
     * Returns a list of bounce proxies that are registered with the bounceproxy
     * controller.
     * 
     * @return
     */
    @GET
    @Produces({ MediaType.APPLICATION_JSON })
    public GenericEntity<List<String>> getBounceProxies() {
        return new GenericEntity<List<String>>(monitoringService.getRegisteredBounceProxies()) {
        };
    }

    /**
     * Reports that a new bounce proxy instance has just been started. <br>
     * The BP instance is either completely new or is re-started after a
     * previous shutdown.
     * 
     * @param bpId
     *            the identifier of the bounce proxy
     * @param urlForCc
     *            the bounce proxy URL used by cluster controllers
     * @param urlForBpc
     *            the bounce proxy URL used by the bounce proxy controller
     * @return
     */
    @PUT
    @Produces({ MediaType.TEXT_PLAIN })
    public Response reportStartup(@QueryParam("bpid") String bpId,
                                  @QueryParam("url4cc") String urlForCc,
                                  @QueryParam("url4bpc") String urlForBpc) {

        if (!monitoringService.isRegistered(bpId)) {

            // first startup
            monitoringService.register(bpId, urlForCc, urlForBpc);
            return Response.created(URI.create(urlForBpc)).build();

        } else {

            // startup after a crash or a shutdown
            monitoringService.reset(bpId, urlForCc, urlForBpc);
            return Response.noContent().build();
        }
    }

    /**
     * Refreshes the status of a bounce proxy instance, e.g. signals a shutdown.
     * 
     * @param bpId
     *            the identifier of the bounce proxy
     * @param statusParam
     *            the status to be reported
     * @return
     */
    @PUT
    @Path("/{bpid: ([A-Z,a-z,0-9,_,\\-]+)(\\.)([A-Z,a-z,0-9,_,\\-]+)}/lifecycle")
    @Produces({ MediaType.TEXT_PLAIN })
    public Response reportShutdown(@PathParam("bpid") String bpId,
                                   @QueryParam("status") BounceProxyStatusParam statusParam) {

        if (BounceProxyStatus.UNRESOLVED.equals(statusParam.getStatus())) {
            // bounce proxy sent an unknown status
            throw new WebApplicationException(Status.BAD_REQUEST);

        } else {
            monitoringService.updateStatus(bpId, statusParam.getStatus());
            return Response.noContent().build();
        }
    }

    /**
     * Refreshes the performance measures of a bounce proxy instance, e.g.
     * active long polls.
     * 
     * @param bpId
     *            the identifier of the bounce proxy
     * @param performanceMap
     *            key-value pairs of performance measures
     * @return
     */
    @POST
    @Path("/{bpid: ([A-Z,a-z,0-9,_,\\-]+)(\\.)([A-Z,a-z,0-9,_,\\-]+)}/performance")
    @Consumes({ MediaType.APPLICATION_JSON })
    @Produces({ MediaType.TEXT_PLAIN })
    public Response reportPerformance(@PathParam("bpid") String bpId, Map<String, Integer> performanceMap) {

        PerformanceMeasures performanceMeasures = new PerformanceMeasures();
        performanceMeasures.addMeasures(performanceMap);

        monitoringService.updatePerformanceMeasures(bpId, performanceMeasures);

        return Response.noContent().build();
    }
}
