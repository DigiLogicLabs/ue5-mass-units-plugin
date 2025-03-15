# Troubleshooting

## Common Issues
- **Units not spawning**: Ensure the Niagara system is properly configured and the spawn function is called correctly.
- **Performance drops**: Check unit counts and adjust LOD and culling settings.
- **Pathing issues**: Verify that the navmesh is up-to-date and covers the playable area.
- **GAS abilities not triggering**: Ensure that the project is correctly set up for GAS and that abilities are properly assigned.

## Debugging Tips
- Use the in-editor debugging tools for Niagara to inspect particle states.
- Monitor GPU and CPU usage to identify bottlenecks.
- Test with smaller unit counts to isolate issues.
- Check the Unreal Engine logs for any GAS-related errors.