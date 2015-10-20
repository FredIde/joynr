package io.joynr.generator.cpp.communicationmodel
/*
 * !!!
 *
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
 *
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
 */

import com.google.inject.Inject
import io.joynr.generator.cpp.util.JoynrCppGeneratorExtensions
import io.joynr.generator.cpp.util.QtTypeUtil
import io.joynr.generator.cpp.util.TemplateBase
import io.joynr.generator.templates.InterfaceTemplate
import io.joynr.generator.templates.util.AttributeUtil
import io.joynr.generator.templates.util.BroadcastUtil
import io.joynr.generator.templates.util.InterfaceUtil
import io.joynr.generator.templates.util.MethodUtil
import io.joynr.generator.templates.util.NamingUtil
import java.util.HashSet
import org.franca.core.franca.FInterface
import org.franca.core.franca.FType

class InterfaceCppTemplate implements InterfaceTemplate{

	@Inject
	private extension JoynrCppGeneratorExtensions

	@Inject
	private extension QtTypeUtil

	@Inject
	private extension NamingUtil

	@Inject
	private extension InterfaceUtil

	@Inject
	private extension MethodUtil

	@Inject
	private extension AttributeUtil

	@Inject
	private extension BroadcastUtil

	@Inject
	private extension TemplateBase

	override generate(FInterface serviceInterface)
'''
«val interfaceName = serviceInterface.joynrName»
«warning()»

#include "«getPackagePathWithJoynrPrefix(serviceInterface, "/")»/I«interfaceName».h"
#include "qjson/serializer.h"
#include "joynr/MetaTypeRegistrar.h"

«FOR parameterType: getRequiredIncludesFor(serviceInterface)»
	#include «parameterType»
«ENDFOR»

#include "joynr/Future.h"

«getNamespaceStarter(serviceInterface)»

I«interfaceName»Base::I«interfaceName»Base()
{
	«val typeObjs = getAllComplexAndEnumTypes(serviceInterface)»
	«var replyMetatypes = getReplyMetatypes(serviceInterface)»
	«var broadcastMetatypes = getBroadcastMetatypes(serviceInterface)»

	«IF !typeObjs.isEmpty() || !replyMetatypes.empty»
		joynr::MetaTypeRegistrar& registrar = joynr::MetaTypeRegistrar::instance();
	«ENDIF»
	«FOR typeobj : typeObjs»
		«val datatype = typeobj as FType»

		// Register metatype «datatype.typeName»
		«IF isEnum(datatype)»
		{
			«registerMetatypeStatement(datatype.typeNameOfContainingClass)»
			int id = «registerMetatypeStatement(datatype.typeName)»
			registrar.registerEnumMetaType<«datatype.typeNameOfContainingClass»>();
			QJson::Serializer::registerEnum(id, «datatype.typeNameOfContainingClass»::staticMetaObject.enumerator(0));
		}
		«ELSE»
			«registerMetatypeStatement(datatype.typeName)»
			registrar.registerMetaType<«datatype.typeName»>();
		«ENDIF»
	«ENDFOR»

	«IF serviceInterface.broadcasts.size > 0»
		/*
		 * Broadcast output parameters are packed into a single publication message when the
		 * broadcast occurs. They are encapsulated in a map. Hence, a new composite data type is
		 * needed for all broadcasts. The map is serialized into the publication message. When
		 * deserializing on consumer side, the right publication interpreter is chosen by calculating
		 * the type id for the composite type.
		*/
	«ENDIF»

	«FOR metatype : replyMetatypes»
		registrar.registerReplyMetaType<«metatype»>();
	«ENDFOR»
	«FOR broadcast: broadcastMetatypes»
		registrar.registerMetaType<«broadcast»>();
	«ENDFOR»
}

const std::string& I«interfaceName»Base::INTERFACE_NAME()
{
	static const std::string INTERFACE_NAME("«getPackagePathWithoutJoynrPrefix(serviceInterface, "/")»/«interfaceName»");
	return INTERFACE_NAME;
}

«getNamespaceEnder(serviceInterface)»
'''
def getReplyMetatypes(FInterface serviceInterface) {
	var replyMetatypes = new HashSet();
	for (method: serviceInterface.methods) {
		if (!method.outputParameters.empty) {
			replyMetatypes.add(method.commaSeparatedOutputParameterTypes)
		}
	}
	for (attribute: serviceInterface.attributes) {
		if (attribute.readable) {
			replyMetatypes.add(attribute.typeName);
		}
	}
	return replyMetatypes;
}
def getBroadcastMetatypes(FInterface serviceInterface) {
	var broadcastMetatypes = new HashSet();
	for (broadcast: serviceInterface.broadcasts) {
		if (!broadcast.outputParameters.empty) {
			broadcastMetatypes.add(broadcast.commaSeparatedOutputParameterTypes)
		}
	}
	return broadcastMetatypes;
}
}