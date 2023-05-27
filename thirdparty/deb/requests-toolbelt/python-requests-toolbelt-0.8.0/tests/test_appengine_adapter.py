# -*- coding: utf-8 -*-
"""Tests for the AppEngineAdapter."""
import sys

import mock
import pytest
import requests

from requests_toolbelt import exceptions as exc

REQUESTS_SUPPORTS_GAE = requests.__build__ >= 0x021000

if REQUESTS_SUPPORTS_GAE:
    from requests.packages.urllib3.contrib import appengine as urllib3_appeng
    from requests_toolbelt.adapters import appengine
else:
    appengine = urllib3_appeng = None


@pytest.mark.skipif(sys.version_info >= (3,),
                    reason="App Engine doesn't support Python 3 (yet) and "
                           "urllib3's appengine contrib code is Python 2 "
                           "only. Until the latter changes, this test will "
                           "be skipped, unfortunately.")
@pytest.mark.skipif(not REQUESTS_SUPPORTS_GAE,
                    reason="Requires Requests v2.10.0 or later")
@mock.patch.object(urllib3_appeng, 'urlfetch')
def test_get(mock_urlfetch):
    """Tests a simple requests.get() call.

    App Engine urlfetch docs:
    https://cloud.google.com/appengine/docs/python/refdocs/google.appengine.api.urlfetch
    """
    response = mock.Mock(status_code=200, content='asdf', headers={})
    mock_urlfetch.fetch = mock.Mock(return_value=response)

    session = requests.Session()
    session.mount('http://', appengine.AppEngineAdapter())
    resp = session.get('http://url/', timeout=9, headers={'Foo': 'bar'})
    assert resp.status_code == 200
    assert resp.content == 'asdf'

    args, kwargs = mock_urlfetch.fetch.call_args
    assert args == ('http://url/',)
    assert kwargs['deadline'] == 9
    assert kwargs['headers']['Foo'] == 'bar'


@pytest.mark.skipif(sys.version_info >= (3,),
                    reason="App Engine doesn't support Python 3 (yet) and "
                           "urllib3's appengine contrib code is Python 2 "
                           "only. Until the latter changes, this test will "
                           "be skipped, unfortunately.")
@pytest.mark.skipif(not REQUESTS_SUPPORTS_GAE,
                    reason="Requires Requests v2.10.0 or later")
def test_appengine_monkeypatch():
    """Tests monkeypatching Requests adapters for AppEngine compatibility.
    """
    adapter = requests.sessions.HTTPAdapter

    appengine.monkeypatch()

    assert requests.sessions.HTTPAdapter == appengine.AppEngineAdapter
    assert requests.adapters.HTTPAdapter == appengine.AppEngineAdapter

    appengine.monkeypatch(validate_certificate=False)

    assert requests.sessions.HTTPAdapter == appengine.InsecureAppEngineAdapter
    assert requests.adapters.HTTPAdapter == appengine.InsecureAppEngineAdapter

    requests.sessions.HTTPAdapter = adapter
    requests.adapters.HTTPAdapter = adapter


@pytest.mark.skipif(sys.version_info >= (3,),
                    reason="App Engine doesn't support Python 3 (yet) and "
                           "urllib3's appengine contrib code is Python 2 "
                           "only. Until the latter changes, this test will "
                           "be skipped, unfortunately.")
@pytest.mark.skipif(not REQUESTS_SUPPORTS_GAE,
                    reason="Requires Requests v2.10.0 or later")
@mock.patch.object(urllib3_appeng, 'urlfetch')
def test_insecure_appengine_adapter(mock_urlfetch):
    adapter = appengine.InsecureAppEngineAdapter()

    assert not adapter._validate_certificate

    with pytest.warns(exc.IgnoringGAECertificateValidation):
        adapter = appengine.InsecureAppEngineAdapter(validate_certificate=True)
