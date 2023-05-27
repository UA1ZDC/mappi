#!/usr/bin/python3

import meteoglobal
import services_pb2
import customviewer_pb2
#    resp = meteoglobal.RemoteCall( servicecode    = services_pb2.kField,
#                                   methodfullname = 'field_pb2@FieldService.GetFieldProto',
#                                   request        = req,
#                                   timeout_msecs  = 30000 )

resp = meteoglobal.RemoteCall( servicecode      = services_pb2.kCustomViewerService,
                               methodfullname   = 'customviewer_pb2@CustomViewerService.GetAvailableTT',
                               request = customviewer_pb2.Dummy(),
                               timeout_msecs = 30000 )

print('RESPA = ', resp )
