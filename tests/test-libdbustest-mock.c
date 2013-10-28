
#include <glib.h>
#include <gio/gio.h>
#include <libdbustest/dbus-test.h>


void
test_basic (void)
{
	DbusTestService * service = dbus_test_service_new(NULL);
	g_assert(service != NULL);

	dbus_test_service_set_conf_file(service, SESSION_CONF);

	DbusTestDbusMock * mock = dbus_test_dbus_mock_new("foo.test");
	g_assert(mock != NULL);

	dbus_test_service_add_task(service, DBUS_TEST_TASK(mock));
	dbus_test_service_start_tasks(service);

	g_assert(dbus_test_task_get_state(DBUS_TEST_TASK(mock)) == DBUS_TEST_TASK_STATE_RUNNING);

	g_object_unref(mock);
	g_object_unref(service);

	return;
}

void
test_properties (void)
{
	DbusTestService * service = dbus_test_service_new(NULL);
	g_assert(service != NULL);

	dbus_test_service_set_conf_file(service, SESSION_CONF);

	DbusTestDbusMock * mock = dbus_test_dbus_mock_new("foo.test");
	g_assert(mock != NULL);

	DbusTestDbusMockObject * obj = dbus_test_dbus_mock_get_object(mock, "/test", "foo.test.interface");
	/* String property */
	g_assert(dbus_test_dbus_mock_object_add_property(mock, obj, "prop1", G_VARIANT_TYPE_STRING, g_variant_new_string("test")));
	/* Invalid type */
	g_assert(!dbus_test_dbus_mock_object_add_property(mock, obj, "prop2", G_VARIANT_TYPE_STRING, g_variant_new_uint32(5)));
	/* Complex type */
	g_assert(dbus_test_dbus_mock_object_add_property(mock, obj, "prop3", G_VARIANT_TYPE("(sssss)"), g_variant_new("(sssss)", "a", "b", "c", "d", "e")));

	dbus_test_service_add_task(service, DBUS_TEST_TASK(mock));
	dbus_test_service_start_tasks(service);

	g_assert(dbus_test_task_get_state(DBUS_TEST_TASK(mock)) == DBUS_TEST_TASK_STATE_RUNNING);

	/* check setup */
	GDBusConnection * bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

	GVariant * propret = NULL;
	GVariant * testvar = NULL;
	GError * error = NULL;

	/* Check prop1 */
	propret = g_dbus_connection_call_sync(bus,
		"foo.test",
		"/test",
		"org.freedesktop.DBus.Properties",
		"Get",
		g_variant_new("(ss)", "foo.test.interface", "prop1"),
		G_VARIANT_TYPE("(v)"),
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
		&error);

	if (error != NULL) {
		g_error("Unable to get property: %s", error->message);
		g_error_free(error);
	}

	g_assert(propret != NULL);
	testvar = g_variant_new_variant(g_variant_new_string("test"));
	g_assert(g_variant_equal(propret, g_variant_new_tuple(&testvar, 1)));

	g_variant_unref(propret);

	/* Check lack of prop2 */
	propret = g_dbus_connection_call_sync(bus,
		"foo.test",
		"/test",
		"org.freedesktop.DBus.Properties",
		"Get",
		g_variant_new("(ss)", "foo.test.interface", "prop2"),
		G_VARIANT_TYPE("(v)"),
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
		&error);

	g_assert(error != NULL);
	g_error_free(error); error = NULL;
	g_assert(propret == NULL);

	/* Check prop3 */
	propret = g_dbus_connection_call_sync(bus,
		"foo.test",
		"/test",
		"org.freedesktop.DBus.Properties",
		"Get",
		g_variant_new("(ss)", "foo.test.interface", "prop3"),
		G_VARIANT_TYPE("(v)"),
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
		&error);

	if (error != NULL) {
		g_error("Unable to get property: %s", error->message);
		g_error_free(error);
	}

	g_assert(propret != NULL);
	testvar = g_variant_new_variant(g_variant_new("(sssss)", "a", "b", "c", "d", "e"));
	g_assert(g_variant_equal(propret, g_variant_new_tuple(&testvar, 1)));

	g_variant_unref(propret);

	/* Clean up */
	g_object_unref(bus);
	g_object_unref(mock);
	g_object_unref(service);

	return;
}

void
test_methods (void)
{
	DbusTestService * service = dbus_test_service_new(NULL);
	g_assert(service != NULL);

	dbus_test_service_set_conf_file(service, SESSION_CONF);

	DbusTestDbusMock * mock = dbus_test_dbus_mock_new("foo.test");
	g_assert(mock != NULL);

	DbusTestDbusMockObject * obj = dbus_test_dbus_mock_get_object(mock, "/test", "foo.test.interface");
	dbus_test_dbus_mock_object_add_method(mock, obj,
		"method1",
		G_VARIANT_TYPE("s"),
		G_VARIANT_TYPE("s"),
		"ret = 'test'");

	dbus_test_service_add_task(service, DBUS_TEST_TASK(mock));
	dbus_test_service_start_tasks(service);

	g_assert(dbus_test_task_get_state(DBUS_TEST_TASK(mock)) == DBUS_TEST_TASK_STATE_RUNNING);

	/* Check 'em */
	GDBusConnection * bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

	GVariant * propret = NULL;
	GVariant * testvar = NULL;
	GError * error = NULL;

	/* Check prop1 */
	propret = g_dbus_connection_call_sync(bus,
		"foo.test",
		"/test",
		"foo.test.interface",
		"method1",
		g_variant_new("(s)", "testin"),
		G_VARIANT_TYPE("(s)"),
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
		&error);

	if (error != NULL) {
		g_error("Unable to call method1: %s", error->message);
		g_error_free(error);
	}

	g_assert(propret != NULL);
	testvar = g_variant_new_string("test");
	g_assert(g_variant_equal(propret, g_variant_new_tuple(&testvar, 1)));

	g_variant_unref(propret);


	/* Clean up */
	g_object_unref(mock);
	g_object_unref(service);

	return;
}


/* Build our test suite */
void
test_libdbustest_mock_suite (void)
{
	g_test_add_func ("/libdbustest/mock/basic",        test_basic);
	g_test_add_func ("/libdbustest/mock/properties",   test_properties);
	g_test_add_func ("/libdbustest/mock/methods",      test_methods);

	return;
}

int
main (int argc, char ** argv)
{
#ifndef GLIB_VERSION_2_36
	g_type_init (); 
#endif

	g_test_init (&argc, &argv, NULL);

	test_libdbustest_mock_suite();

	g_log_set_always_fatal(G_LOG_LEVEL_ERROR);

	return g_test_run();
}
