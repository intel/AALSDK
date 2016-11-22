import ctypes

def testit():
    sm = ctypes.CDLL("cva.so")
    sm.service_manager_create_services("services.json")
    svc = sm.service_manager_get_client("PR")
    #sm.reconfigure_service_reconfigure(svc, "/home/rrojo/mode0.rbf")
    #sm.service_manager_shutdown()

if __name__ == "__main__":
    testit()

