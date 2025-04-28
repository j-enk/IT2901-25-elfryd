<template>
    <svg ref="svgRef" :width="width" :height="height"></svg>
</template>

<script setup lang="ts">
import { ref, onMounted, watch } from 'vue';
import * as d3 from 'd3';

const props = defineProps<{
    data: Array<{
        battery_id: number;
        voltage: number;
        date: Date;
    }>;
}>();

const svgRef = ref<SVGSVGElement | null>(null);
const width = 800;
const height = 500;
const margin = { top: 20, right: 30, bottom: 30, left: 40 };

const drawChart = () => {
    if (!svgRef.value) return;

    const svg = d3.select(svgRef.value);
    svg.selectAll('*').remove(); // Clear previous contents


    const innerWidth = width - margin.left - margin.right;
    const innerHeight = height - margin.top - margin.bottom;

    const g = svg.append('g')
        .attr('transform', `translate(${margin.left},${margin.top})`);

    // Group data by battery_id
    const grouped = d3.group(props.data, d => d.battery_id);

    // Define scales
    const xScale = d3.scaleTime()
        .domain(d3.extent(props.data, d => d.date))
        .range([0, innerWidth]);

    const yScale = d3.scaleLinear()
        .domain([0, d3.max(props.data, d => d.voltage)])
        .nice()
        .range([innerHeight, 0]);

    // Define axes
    const xAxis = d3.axisBottom(xScale)
        .ticks(6)
        .tickFormat(d3.timeFormat('%Y-%m-%d %H:%M'));
    const yAxis = d3.axisLeft(yScale);

    // Append axes
    g.append('g')
        .attr('transform', `translate(0,${innerHeight})`)
        .call(xAxis)
        .selectAll('text')
        .attr('transform', 'rotate(-45)')
        .style('text-anchor', 'end');

    g.append('g').call(yAxis);

    // Define line generator
    const line = d3.line()
        .x(d => xScale(d.date))
        .y(d => yScale(d.voltage));

    // Define color scale
    const color = d3.scaleOrdinal(d3.schemeCategory10);

    // Draw lines
    Array.from(grouped.entries()).forEach(([batteryId, values], index) => {
        g.append('path')
            .datum(values)
            .attr('fill', 'none')
            .attr('stroke', color(index.toString()))
            .attr('stroke-width', 1.5)
            .attr('d', line);
    });

    // Add legend group
    const legend = svg.append('g')
        .attr('class', 'legend')
        .attr('transform', `translate(${width - margin.right - 150}, ${margin.top})`);

    // Legend data
    const legendItems = Array.from(grouped.keys());
    const legendItemHeight = 20;
    const legendPadding = 15;
    const legendWidth = 100;
    const legendHeight = legendItems.length * legendItemHeight + legendPadding;

    // Add background rectangle
    legend.append('rect')
        .attr('width', legendWidth)
        .attr('height', legendHeight)
        .attr('fill', '#000') // Tailwind's bg-base-300 approx: slate-300
        .attr('rx', 4)
        .attr('ry', 4);

    // Add each legend row
    legendItems.forEach((batteryId, index) => {
        const legendRow = legend.append('g')
            .attr('transform', `translate(${legendPadding / 2}, ${index * legendItemHeight + legendPadding / 2})`);

        legendRow.append('rect')
            .attr('width', 12)
            .attr('height', 12)
            .attr('fill', color(index.toString()));

        legendRow.append('text')
            .attr('x', 18)
            .attr('y', 10)
            .style('font-size', '12px')
            .style('fill', '#f9f9f9')
            .text(`Battery ${batteryId}`);
    });

};


onMounted(drawChart);
watch(() => props.data, drawChart);
</script>
