import React from "react";

export default class ErrorBoundary extends React.Component<
  { children: React.ReactNode },
  { error?: Error }
> {
  state = { error: undefined as Error | undefined };

  static getDerivedStateFromError(error: Error) {
    return { error };
  }

  componentDidCatch(error: Error, info: React.ErrorInfo) {
    console.error("UI_PANEL_ERROR", error, info);
  }

  render() {
    if (this.state.error) {
      return (
        <div className="m-3 p-3 rounded-lg border border-red-600/60 bg-red-950/40 text-red-200 text-sm">
          Panel error: {this.state.error.message}
        </div>
      );
    }
    return this.props.children;
  }
}
