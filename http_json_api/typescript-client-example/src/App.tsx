import { useEffect, useState } from 'react';
import { NavClient, ProcessorSettings } from './nav_client';

import './App.css'

const navClient = new NavClient(
  {
    BASE: 'http://localhost:3000',
  }
);

function App() {

  const [processorSettings, setProcessorSettings] =
    useState<ProcessorSettings>();

  useEffect(() => {
    navClient.default.processorSettingsApiProcessorSettingsGet()
      .then((response) => {
        setProcessorSettings(response);
      })
      .catch((error) => {
        console.log(error);
      });
  }, []);

  return (
    <main>
      <h1>Nav Client</h1>
      <section>
        <h2>Processor Settings</h2>
        <h3>
          {processorSettings
          ? "Response from SonaSoft!"
          : "No response from SonaSoft :("}
        </h3>
        <div>
          {processorSettings
          ? Object.keys(processorSettings || {}).map((key) => {
              return (
                <div key={key}>
                  <span>{key}:{" "}</span>
                  <span>{processorSettings?.[key as keyof ProcessorSettings]}</span>
                </div>
              )
            })
          : "Loading..."}
        </div>
      </section>
    </main>
  )
}

export default App
